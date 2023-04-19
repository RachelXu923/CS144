#include "wrapping_integers.hh"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>

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

  uint32_t q = checkpoint / (1UL << 32);
  uint32_t remain = checkpoint % (1UL << 32);
  uint32_t offset = raw_value_ - zero_point.raw_value_;

  if (offset > remain && (offset - remain) > (1UL<<32) - (offset - remain) && q > 0){
    q--;
  }
  if (offset < remain && (remain - offset) > (1UL<<32)-((remain - offset)) && q < UINT32_MAX){
    q++;
  }
  return q*(1UL<<32) + offset;

  

  // Wrap32 check = wrap(checkpoint, zero_point);
  // uint32_t dist = 0;
  // uint64_t ans = 0;
  // if(zero_point.raw_value_ == 19){
  //   cout << "checkpoint: " << checkpoint<< endl;
  //   cout << "check raw value: " << check.raw_value_<< endl;
  //   cout << "raw value: " << raw_value_ << endl;
  // }
  // if (check.raw_value_ > raw_value_){
  //   dist = check.raw_value_ - raw_value_;
  //   if (dist < checkpoint){
  //     ans = checkpoint - dist;
  //   }else{
  //     ans = checkpoint + (1UL << 32) -dist;
  //   }
  // }else if(check.raw_value_ < raw_value_){
  //   dist = raw_value_ - check.raw_value_;
  //   ans = checkpoint + dist;
  // }else if(check.raw_value_ == raw_value_){
  //   return checkpoint;
  // }
  // if (ans + (1UL << 32) - checkpoint < dist){
  //   ans += (1UL << 32);
  //   return ans;
  // } 
  // if (checkpoint - (ans - (1UL << 32)) < dist && ans >= (1UL << 32)){
  //   ans -= (1UL << 32);
  //   return ans;
  // }
  // return ans;
}
