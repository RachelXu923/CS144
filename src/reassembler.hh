#pragma once

#include "byte_stream.hh"

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <sys/types.h>

class Reassembler
{
public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  Reassembler() = default;

  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  uint64_t bytes_pending() const;

protected:
  uint64_t _tail = 0;        // mark the end the buffer
  uint64_t _pending_cnt = 0; // bytes still pending in the map, received but not written yet
  std::map<uint64_t, std::string> _map
    = {}; //<key: first_index, value: data>: store the data block(key is their first index)
  void merge_string( uint64_t first_index, std::string data ); // merge new data block into the map
  bool _eof = false;                                           // mark if last string was received
  void map_insert( uint64_t first_index, std::string data );   // insert data into the map manage pending cnt
  void merge_helper( uint64_t first_index, std::string data, auto left ); // helper function of merge
};
