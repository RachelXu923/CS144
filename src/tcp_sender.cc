#include "tcp_sender.hh"
#include "buffer.hh"
#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"

#include <cstddef>
#include <cstdint>
#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}



uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return sequence_number_in_flight;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_cnt;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  uint16_t cur_capacity = window_size > 0? window_size : 1;
  while (cur_capacity > sequence_number_in_flight + pending_size && !FIN && ){
    uint16_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, cur_capacity);
    TCPSenderMessage seg;
    //if no SYN, add isn to syn
    if (!SYN){
      seg.SYN = true;
      SYN = true;
      payload_size --;
    }
    seg.seqno = Wrap32::wrap(next_ackno, isn_);
    string payload;
    read(outbound_stream, payload_size, payload);
    seg.payload = Buffer(payload);
    if (outbound_stream.is_finished() && sequence_number_in_flight + payload_size + seg.SYN < cur_capacity){
      seg.FIN = true;
      FIN = true;
    }
    pending_seg_.push(seg);
    pending_size += seg.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  uint64_t abs_ackno = msg.ackno->unwrap(isn_, next_ackno);
  if (abs_ackno > next_ackno){
    return;
  }
  next_ackno = abs_ackno;
  window_size = msg.window_size;
  while (!outstanding_seg_.empty() && outstanding_seg_.front().seqno.unwrap(isn_, next_ackno) <= next_ackno){
    sequence_number_in_flight -= outstanding_seg_.front().sequence_length();
    outstanding_seg_.pop();
    if(is_running){
      end_timer();
    }
  }
  if (!outstanding_seg_.empty()){
    start_timer();
  }
  consecutive_retransmissions_cnt = 0;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  if(is_running){
    update_timer(ms_since_last_tick);
  }else{
    start_timer();
  }
  if (is_expire()){
    //retransmit the ealiest
    maybe_send();
    if(window_size > 0){
      double_RTO();
      consecutive_retransmissions_cnt ++;
    }
    reset_timer();
  }
}

void TCPSender::start_timer(){
  is_running = true;
  current_time = 0;
  cnt = 1;
}
void TCPSender::update_timer(size_t ms_since_last_tick){
  current_time += ms_since_last_tick;
}

void TCPSender::double_RTO(){
  cnt *= 2;
}

bool TCPSender::is_expire(){
  return initial_RTO_ms_ * cnt > current_time;
}

void TCPSender::reset_timer(){
  is_running = true;
  current_time = 0;
}
