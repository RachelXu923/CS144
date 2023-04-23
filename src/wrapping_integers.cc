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
  // abs seqno(64) -> seqno(32)

  return zero_point.operator+( static_cast<uint32_t>( n ) );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  // return {};
  // convert seqno(Wrap32) -> absolute seqno(uint64_t)

  // distance between checkpoint and zero point
  uint32_t check = checkpoint % ( 1UL << 32 );
  // distance between x and zero point
  // zero point can either on the left or right
  uint32_t dist = raw_value_ - zero_point.raw_value_;
  // checkpoint can either on the left or right
  // obtained candidate seqno based on the offset
  // the “absolute” offset between checkpoint and x equals "seqno" offset between checkpoint and x
  uint64_t ans = checkpoint + dist - check;

  // first term:handle int overflow
  // secend term: find the closest point
  if ( ans + ( 1UL << 32 ) > checkpoint && ans + ( 1UL << 32 ) - checkpoint < ( check - dist ) ) {
    ans += ( 1UL << 32 );
  } else if ( ans - ( 1UL << 32 ) < checkpoint && checkpoint - ( ans - ( 1UL << 32 ) ) < ( dist - check ) ) {
    ans -= ( 1UL << 32 );
  }
  return ans;
}
