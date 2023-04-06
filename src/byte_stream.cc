#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), is_closed_(false), if_error_(false), buffer_(""), cnt(0) {}

void Writer::push( string data )
{
  // Your code here.
  uint64_t r_capacity = available_capacity();
  // if (r_capacity == 0){
  //   return;
  // }
  if (r_capacity >= data.length()){
    // data_ += data;
    buffer_ += data;
    cnt += data.length();
  }else{
    // data_.append(data, 0, r_capacity);
    buffer_.append(data, 0, r_capacity);
    cnt += r_capacity;
  }
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  if_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  uint64_t remain  = capacity_ - buffer_.length();
  return remain;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  // return data_.length();
  return cnt;
}

string_view Reader::peek() const
{
  // Your code here.
  // uint64_t p_len = 0;
  // if(buffer_.length() > 0){
  //   p_len = 1;
  // }
  string_view view(buffer_.c_str(), buffer_.length());
  return view;
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && buffer_.length() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return if_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  // uint64_t pop_len = 0;
  // if(len < buffer_.length()){
  //   pop_len = buffer_.length() - len;
  // }
  // buffer_ = buffer_.substr(len, pop_len);
  
  // modified according to ed discussion #94
  buffer_.erase(0, len);
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.length();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  
  // return data_.length() - buffer_.length();
  return cnt - buffer_.length();
}
