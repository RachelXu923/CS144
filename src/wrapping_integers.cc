#include "wrapping_integers.hh"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sys/types.h>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // (void)n;
  // (void)zero_point;
  return zero_point.operator+(static_cast<uint32_t>(n));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  // return {};
  //convert seqno(Wrap32) -> absolute seqno(uint64_t)

  // uint32_t q = checkpoint / (1UL << 32);
  // uint32_t remain = checkpoint % (1UL << 32);
  // uint32_t offset = raw_value_ - zero_point.raw_value_;

  // if (offset > remain && (offset - remain) > (1UL<<32) - (offset - remain) && q > 0){
  //   q--;
  // }
  // if (offset < remain && (remain - offset) > (1UL<<32)-((remain - offset)) && q < UINT32_MAX){
  //   q++;
  // }
  // return q*(1UL<<32) + offset;

  uint32_t check = checkpoint % (1UL << 32);
  uint32_t abs_seq = raw_value_ - zero_point.raw_value_;
  uint64_t ans = checkpoint + abs_seq - check;

  if (ans + (1UL << 32) > checkpoint && ans + (1UL<<32) - checkpoint < (check - abs_seq)){
    ans += (1UL << 32);
  }else if (ans - (1UL << 32) < checkpoint && checkpoint - (ans - (1UL << 32)) < (abs_seq - check)){
    ans -= (1UL << 32);
  }
  return ans;
}
