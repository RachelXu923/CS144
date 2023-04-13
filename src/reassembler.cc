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
  //1. discard the tail beyond capacity  
  //2. truncate the head if first index smaller than the buffer tail
  //3. merge the data into the map
  //4. pop the data from the map and write them into the bytestream until there's a gap in the maplist
  //5. end of file: last string received && map is empty
  uint64_t last_index = first_index + data.length();

  // truncate tail
  if ( last_index >= _tail + output.available_capacity() ) {
    uint64_t out = last_index - ( _tail + output.available_capacity() );
    uint64_t pos = data.length() - out;
    data.erase( pos, out );
  }
  //truncate head
  if ( first_index <= _tail ) {
    data.erase( 0, _tail - first_index );
    first_index = _tail;
  }
  //merge data
  merge_string( first_index, data );

  //write the data
  auto iter = _map.begin();
  while ( iter != _map.end() && iter->first == _tail ) {
    output.push( iter->second );
    _tail += iter->second.length();
    _pending_cnt -= iter->second.length();
    _map.erase( iter++ );
  }
  //see if eof
  if ( is_last_substring ) {
    _eof = true;
  }
  if ( _eof && _map.empty() ) {
    output.close();
  }
}

//helper function for merge: insert data into the map and manage the pending byte
void Reassembler::map_insert( uint64_t first_index, string data )
{
  if ( data.length() >= 1 ) {
    _map[first_index] = data;
    _pending_cnt += data.length();
  }
}

//helper function for merge blocks: 
// 1. delete all the <K, V> in the map that was totally covered by the new data
// 2. if new data has overlap with the original <K, V>, truncate the new data 
// 3. insert the new data to the map
void Reassembler::merge_helper( uint64_t first_index, string data, auto left )
{
  //delete all the old data that is totally covered by the new data using while loop
  while ( left != _map.end() && left->first + left->second.length() <= first_index + data.length() ) {
    _pending_cnt -= left->second.length();
    _map.erase( left++ );
  }
  //if map is end, insert
  if ( left == _map.end() ) {
    map_insert( first_index, data );
  } else {
    //if new data has no overlap, insert
    if ( left->first >= first_index + data.length() ) {
      map_insert( first_index, data );
    } else {
      //if overlap with old data, truncate the new string
      uint64_t len = first_index + data.length() - left->first;
      uint64_t pos = data.length() - len;
      data.erase( pos, len );
      map_insert( first_index, data );
    }
  }
}
//merge function: merge the new data into the map <key: first index, value: data>
// 1. if map is empty, insert
// 2. using lower_bound to find the closest key >= firstindex
// 2.1 closest key is at the begining 
// 2.2 closest key at the end
// 2.3 closest key in the middle
void Reassembler::merge_string( uint64_t first_index, string data )
{
  if ( _map.empty() ) {
    map_insert( first_index, data );
    return;
  }
  uint64_t last_index = first_index + data.length();
  auto left = _map.lower_bound( first_index );
  //case 2.1
  if ( left == _map.begin() ) {
    //no overlap, insert
    if ( last_index <= left->first ) {
      map_insert( first_index, data );
    } else if ( last_index > left->first && last_index < left->first + left->second.length() ) {
      //part of the data overlap, truncate the tail of the new data and insert
      uint64_t len = last_index - left->first;
      uint64_t pos = data.length() - len;
      data.erase( pos, len );
      map_insert( first_index, data );
      return;
    } else {
      //closest data block was totally covered by the new data
      //use helper function to delete all the fully-covered old data and truncate new data to fit in
      merge_helper( first_index, data, left );
    }
  } else if ( left == _map.end() ) {
    // case 2.2 new data has the largest key
    // therefore we find the last iterator
    left--;
    uint64_t r_idx = left->first + left->second.length();
    //if no overlap with the last block, insert
    if ( r_idx <= first_index ) {
      map_insert( first_index, data );
    } else if ( r_idx > first_index && r_idx <= last_index ) {
      //if part of the data overlapped with the last block, truncate and insert
      uint64_t len = r_idx - first_index;
      data.erase( 0, len );
      map_insert( r_idx, data );
      return;
    }
  } else {
    // case 2.3 new data need to be merged in the middle of the map
    // same as in 2.2, find the last iterator
    left--;
    //if overlapped with the last block, truncate the head
    if ( left->first + left->second.length() > first_index ) {
      uint64_t len = left->first + left->second.length() - first_index;
      data.erase( 0, len );
      first_index += len;
    }
    //then use helper function to handle the fully-covered case
    left++;
    merge_helper( first_index, data, left );
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return _pending_cnt;
}
