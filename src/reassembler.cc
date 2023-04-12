#include "reassembler.hh"
#include "byte_stream.hh"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;

  uint64_t last_index = first_index + data.length();

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
    merge_string(first_index, data);
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
  if(is_last_substring){
    _eof = true;
  }
  if(_eof && _map.empty()){
    output.close();
  }
}
void Reassembler::map_insert(uint64_t first_index, string data){
  if (data.length() >=1){
    _map[first_index] = data;
    _pending_cnt += data.length();
  }
}
void Reassembler::merge_helper(uint64_t first_index, string data, auto left){
  while (left != _map.end() && left->first + left->second.length() <= first_index + data.length()){
    _pending_cnt -= left->second.length();
    _map.erase(left++);
  }
  if(left == _map.end()){
    map_insert(first_index, data);
  }else{
    if (left->first >= first_index + data.length()){
      map_insert(first_index, data);
    }else{
      uint64_t len = first_index + data.length() - left->first;
      uint64_t pos = data.length() - len;
      data.erase(pos, len);
      map_insert(first_index, data);
    }
  }
}
void Reassembler::merge_string(uint64_t first_index, string data){
  if (_map.empty()){
    map_insert(first_index, data);
    return;
  }
  uint64_t last_index = first_index + data.length();
  auto left = _map.lower_bound(first_index);
  if(left == _map.begin()){
    if (last_index <= left->first){
      map_insert(first_index, data);
    }else if (last_index > left->first && last_index < left->first + left->second.length()){
      uint64_t len = last_index - left->first;
      uint64_t pos = data.length() - len;
      data.erase(pos, len);
      map_insert(first_index, data);
      return;
    }else{
      merge_helper(first_index, data, left);
    }
  }else if(left == _map.end()){
    left--;
    uint64_t r_idx = left->first + left->second.length();
    if(r_idx <= first_index){
      map_insert(first_index, data);
    }else if(r_idx > first_index && r_idx <= last_index){
      //need to truncate 
      uint64_t len = r_idx - first_index;
      data.erase(0, len);
      map_insert(r_idx, data);
      return;
    }
  }else{
    left--;
    if (left->first + left->second.length() > first_index){
      uint64_t len = left->first + left->second.length() - first_index;
      data.erase(0, len);
      first_index += len;
    }
    left++;
    merge_helper(first_index, data,left);
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return _pending_cnt;
}
