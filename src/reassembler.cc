#include "reassembler.hh"
#include "byte_stream.hh"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

// Reassembler::Reassembler();

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;
  //delete the prefix stream
  //merge the substring
  //push into the Bytestream
  //EOF

  //case 1: data can write to the bytestream begin(firstindex) <= head && end >head
  //truncate overlap prefix
  // 1.1 nothing to merge in the map -> write
  // 1.2 need to merge string in the map -> merge string -> truncate overlap(head) -> write
  //case 2: data cannot be write: first index > head 
  // 2.1 nothing overlap in the map -> add to the map <first index, data>
  // 2.2 head overlap -> add new string to the map
  // 2.3 tail overlap -> add new string to the map
  uint64_t last_index = first_index + data.length();
  // if (last_index <= _tail){
    
  // }

  //truncate tail
  if (last_index >= _tail + output.available_capacity()){
    uint64_t out = last_index - (_tail + output.available_capacity());
    uint64_t pos = data.length() - out;
    data.erase(pos, out);
  }

  //case 1
  if (first_index <= _tail){
    //truncate head
    data.erase(0, _tail - first_index);
    first_index = _tail;
    //merge check
    // auto iter = _map.upper_bound(_tail);
    // if (iter != _map.end() && iter->first > last_index){
    //   output.push(data);
    //   _tail += data.length();
    // }else{
      merge_string(first_index, data);
    // }
  }else{
    merge_string(first_index, data);
  }
  auto iter = _map.lower_bound(_tail);

  while (iter != _map.end() && iter->first == _tail){
    output.push(iter->second);
    _tail += iter->second.length();
    _pending_cnt -= iter->second.length();
    _map.erase(iter++);
  }
  // while (iter->first == _tail){
  //   output.push(iter->second);
  //   _tail += iter->second.length();
  //   iter++;
  // }
  if(is_last_substring){
    _eof = true;
  }
  if(_eof && _map.empty()){
    output.close();
  }
}

void Reassembler::merge_string(uint64_t first_index, string data){
  // //manage bytes pending
  // // auto iter = _map.upper_bound(first_index);
  // uint64_t last_index = first_index +data.length();
  // //data'index is largest
  // if (_map.upper_bound(first_index) == _map.end()){
  //   auto iter = _map.end();
  //   iter--;
  //   if (iter->first + iter->second.length() < last_index){
  //     uint64_t len = iter->first + iter->second.length() - first_index;
  //     data.erase(0, len);
  //   }
  // }else{
  //   //data is not the largest key
  //   auto left = _map.lower_bound(first_index);
  //   auto right  = _map.lower_bound(last_index);
  //   //truncate left if overlap
  //   if (left != _map.begin()){
  //     auto ll = --left;
  //     data.erase(0, ll->first + ll->second.length() - first_index);
  //     first_index += ll->first + ll->second.length();
  //   }
  //   if(left->first != right->first){
      
  //   }



  uint64_t l = first_index;
  uint64_t r = first_index + data.length();
  auto left = _map.lower_bound(l);
  if (left != _map.begin()){
    left--;
  }
  if (left->first + left->second.length() > l){
    left->second.erase(first_index - left->first, left->first + left->second.length() - first_index);
    _pending_cnt -= left->first + left->second.length() - first_index;
  }
  left ++;
  while (left != _map.end() && left->first + left->second.length() <= r){
    _pending_cnt -= left->second.length();
    _map.erase(left);
    left = _map.lower_bound(l);
  }

  if (left != _map.end() && left->first < r){
    _map[r] = left->second.erase(0, r - left->first);
    _pending_cnt += _map[r].length();
    _map.erase(left);
    _pending_cnt -= left->second.length();
  }

  if (data.length() > 0){
    _map[first_index] = data;
    // _map.insert(map<uint64_t, string>::value_type(first_index, data));
    _pending_cnt += data.length();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return _pending_cnt;
}
