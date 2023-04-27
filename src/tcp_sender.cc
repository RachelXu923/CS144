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
  optional<TCPSenderMessage> output;
  if (!pending_seg_.empty()){
    TCPSenderMessage seg;
    seg = pending_seg_.front();
    pending_seg_.pop_front();
    outstanding_seg_.push_back(seg);
    // cout << "add seg to outstanding" << endl; 
    output = seg;
    next_ackno += seg.sequence_length();
  }
  return output;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  uint16_t cur_capacity = window_size > 0? window_size : 1;
  // cout << "window size: " << window_size<< endl;
  // cout << "cur capacity: " << cur_capacity << endl;
  while (cur_capacity > sequence_number_in_flight && !FIN ){
    uint16_t payload_size = TCPConfig::MAX_PAYLOAD_SIZE;
    if (cur_capacity - sequence_number_in_flight < payload_size){
      payload_size = cur_capacity - sequence_number_in_flight;
    }
    cout << "payload size"<< payload_size << endl; 
    TCPSenderMessage seg;
    //if no SYN, add isn to syn
    if (!SYN){
      // cout << "set SYN" << endl;
      seg.SYN = true;
      cout << seg.SYN << endl;
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
    if (seg.sequence_length() == 0){
      return;
    }
    pending_seg_.push_back(seg);
    // next_ackno += seg.sequence_length();
    if (!is_running){
      start_timer();
    }
    sequence_number_in_flight += seg.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage seg;
  seg.seqno = Wrap32::wrap(next_ackno, isn_);
  return seg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if(!msg.ackno.has_value()){
    cout << "ackno has no value" << endl;
    return;
  }
  uint64_t abs_ackno = msg.ackno->unwrap(isn_, next_ackno);

  if (abs_ackno > next_ackno){
    return;
  }

  // cout << "start new ack"<< endl;
  // cout << "abs ackno: " << abs_ackno << endl;
  // cout << "next ackno: " << abs_ackno <<endl;
  // cout << "outstanding q len: " << outstanding_seg_.size() << endl;
  window_size = msg.window_size;
  while (!outstanding_seg_.empty()){
    TCPSenderMessage seg = outstanding_seg_.front();
    cout << "cur ackno: " << seg.seqno.unwrap(isn_, next_ackno) <<endl;
    if (seg.seqno.unwrap(isn_, next_ackno) < abs_ackno){
      // cout << "drop seg" << endl;
      // cout << "seq length: " << seg.sequence_length() << endl;
      // cout << "in flight: " << sequence_number_in_flight << endl;
      sequence_number_in_flight -= (seg.sequence_length());
      outstanding_seg_.pop_front();
      cout << "outstanding q len: " << outstanding_seg_.size() << endl;
      if(is_running){
        end_timer();
      }     
    }else{
      cout << "end pop" << endl;
      break;
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
    cout << "current time" << current_time << endl;
  }else{
    start_timer();
  }
  if (is_expire() && !outstanding_seg_.empty()){
    //retransmit the ealiest
    cout << "expire" << endl;
    pending_seg_.push_back(outstanding_seg_.front());
    next_ackno -= outstanding_seg_.front().sequence_length();
    outstanding_seg_.pop_front();
    if(window_size > 0){
      double_RTO();
      consecutive_retransmissions_cnt ++;
    }
    reset_timer();
  }
  if(outstanding_seg_.empty()){
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
  // cout << "cnt: " << cnt<< endl;
  // cout << "rto: " << initial_RTO_ms_ * cnt << endl;
  return initial_RTO_ms_ * cnt <= current_time;
}

void TCPSender::reset_timer(){
  is_running = true;
  current_time = 0;
}
