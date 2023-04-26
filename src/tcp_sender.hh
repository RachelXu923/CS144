#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <queue>

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick ); //wait for too long re-transmitting the oldest 

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?

private:
  std::queue<TCPSenderMessage> outstanding_seg_{};
  std::queue<TCPSenderMessage> pending_seg_{};
  bool SYN = false;
  bool FIN = false;
  uint16_t window_size = 0;
  uint64_t next_ackno = 0;
  uint64_t sequence_number_in_flight = 0;
  uint64_t pending_size = 0;
  uint64_t consecutive_retransmissions_cnt = 0;

  bool is_running = false;
  size_t current_time = 0;
  uint64_t cnt = 1;

  void start_timer();
  void end_timer();
  void update_timer(uint64_t ms_since_last_tick);
  void double_RTO();
  bool is_expire();
  void reset_timer();
};

