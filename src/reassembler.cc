#include "reassembler.hh"

using namespace std;

Reassembler::Reassembler( uint64_t capacity )
  : _capacity( capacity )
{}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;
  //if capacity is zero -> return
  //redundant substring deletion
  //merge the substring
  //push into the Bytestream
  //EOF
  
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return _pending_cnt;
}
