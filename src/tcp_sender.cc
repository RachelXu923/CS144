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
  return sequence_number_in_flight;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_cnt;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  optional<TCPSenderMessage> output;
  //send msg only when there's still segments not sent yet
  if (!pending_seg_.empty()){
    TCPSenderMessage seg;
    //send the first msg in the queue then pop it
    seg = pending_seg_.front();
    pending_seg_.pop_front();
    //add this msg (just sent) to the outstanding queue based on its seqno
    if (seg.seqno.unwrap(isn_, next_seqno) < outstanding_seg_.front().seqno.unwrap(isn_, next_seqno)){
      outstanding_seg_.push_front(seg);
    }else{
      outstanding_seg_.push_back(seg);
    }
    output = seg;
    //update seqno based on the sent msg length
    next_seqno += seg.sequence_length();
  }
  //if pending queue is empty, directly return an empty output
  return output;
}

void TCPSender::push( Reader& outbound_stream )
{
  uint16_t cur_capacity = window_size > 0? window_size : 1;
  //set a temperary sequence number in case reading multiple msgs (as next seqno only changes in maybe send)
  uint64_t tmp_seqno = next_seqno;
  //only push when there's still space for the new msg or fin not sent yet
  while (cur_capacity > sequence_number_in_flight && !FIN ){
    //make sure the payload size if no larger than max payload size
    uint16_t payload_size = TCPConfig::MAX_PAYLOAD_SIZE;
    if (cur_capacity - sequence_number_in_flight < payload_size){
      payload_size = cur_capacity - sequence_number_in_flight;
    }
    TCPSenderMessage seg;
    //send syn
    if (!SYN){
      seg.SYN = true;
      SYN = true;
      payload_size --;
    }
    //read from reader based on the tmp_seqno and payload size
    seg.seqno = Wrap32::wrap(tmp_seqno, isn_);
    string payload;
    read(outbound_stream, payload_size, payload);
    seg.payload = Buffer(payload);
    //if stream is finished, send fin when there's space
    if (outbound_stream.is_finished() && sequence_number_in_flight + seg.sequence_length() < cur_capacity){
      seg.FIN = true;
      FIN = true;
    }
    //if this is an empty msg return
    if (seg.sequence_length() == 0){
      return;
    }
    //add msg to the back of the pending queue and wait to be sent
    pending_seg_.push_back(seg);
    tmp_seqno += seg.sequence_length();
    if (!is_running){
      start_timer();
    }
    sequence_number_in_flight += seg.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage seg;
  //send a msg that sequence number = next_seqno
  seg.seqno = Wrap32::wrap(next_seqno, isn_);
  return seg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  //update window size based on the msg
  window_size = msg.window_size;
  uint64_t abs_ackno = msg.ackno->unwrap(isn_, next_seqno);

  //invalid ackno msg
  if (abs_ackno > next_seqno){
    return;
  }
  // drop the outstanding msg that has been acked
  while (!outstanding_seg_.empty()){
    TCPSenderMessage seg = outstanding_seg_.front();
    if (seg.seqno.unwrap(isn_, next_seqno) + seg.sequence_length() <= abs_ackno){
      sequence_number_in_flight -= (seg.sequence_length());
      outstanding_seg_.pop_front();
      //stop timer when there's new data being acked
      if(is_running){
        end_timer();
      }     
    }else{
      break;
    }
  }
  //if there's still outstanding msg, restart the timer
  if (!is_running && !outstanding_seg_.empty()){
    start_timer();
  }
  //update consecutive retransmission count
  consecutive_retransmissions_cnt = 0;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  //if timer is running, update the current time or start the timer
  if(is_running){
    update_timer(ms_since_last_tick);
  }else{
    start_timer();
  }
  //if timer has expired and outstanding is not empty
  //retransmit the msg by remove it from the outstanding queue and insert it to the pending queue so that it could be resent
  //make sure to add it to the back or front based on its seqno
  if (is_expire() && !outstanding_seg_.empty()){
    if (outstanding_seg_.front().seqno.unwrap(isn_, next_seqno) < pending_seg_.front().seqno.unwrap(isn_, next_seqno)){
      pending_seg_.push_front(outstanding_seg_.front());
    }else{
      pending_seg_.push_back(outstanding_seg_.front());
    }
    reset_timer();
    next_seqno -= outstanding_seg_.front().sequence_length();
    outstanding_seg_.pop_front();
    //double rto only when the window is nonzero
    if(window_size > 0){
      double_RTO();
      consecutive_retransmissions_cnt ++;
    }
  }
  //if all outstanding data has been acked, stop the timer
  if(outstanding_seg_.empty() && pending_seg_.empty()){
    end_timer();
  }
}
void TCPSender::start_timer(){
  is_running = true;
  current_time = 0;
  cnt = 1;
}
void TCPSender::end_timer(){
  is_running = false;
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
  return current_time >= initial_RTO_ms_ * cnt;
}

void TCPSender::reset_timer(){
  is_running = true;
  current_time = 0;
}
