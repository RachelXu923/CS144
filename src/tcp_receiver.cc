#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  // (void)message;
  // (void)reassembler;
  // (void)inbound_stream;

  // if syn flag is false and message does not include syn flag, return
  if ( !SYN && !message.SYN ) {
    return;
  }

  uint64_t abs_seqno = 0;
  // if this message contains SYN, then its seqno is ISN and seqno of first byte is ISN+1
  // therefore abs_seqno = seqno's abs seqno+1
  if ( message.SYN ) {
    SYN = true;
    ISN = message.seqno;
    abs_seqno = message.seqno.unwrap( ISN, checkpoint ) + 1;
  } else {
    // else, seqno in the message is the seqno of first byte
    abs_seqno = message.seqno.unwrap( ISN, checkpoint );
  }
  // insert payload to the ressembler
  // stream index = abs seqno-1
  string str = message.payload.release();
  reassembler.insert( abs_seqno - 1, str, message.FIN, inbound_stream );
  checkpoint += str.size();
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  // (void)inbound_stream;

  TCPReceiverMessage msg;
  // the maxinum value of window size is UINT16_MAX
  if ( inbound_stream.available_capacity() > UINT16_MAX ) {
    msg.window_size = UINT16_MAX;
  } else {
    msg.window_size = inbound_stream.available_capacity();
  }
  // ackno is optional
  // if SYN flag is false, directly return msg and leave ackno empty
  if ( !SYN ) {
    return msg;
  }
  // SYN + x bytes have been pushed into the writer
  uint64_t written = inbound_stream.bytes_pushed() + 1;
  // if closed, FIN also need one byte
  if ( inbound_stream.is_closed() ) {
    written++;
  }
  msg.ackno = Wrap32::wrap( written, ISN );
  return msg;
}
