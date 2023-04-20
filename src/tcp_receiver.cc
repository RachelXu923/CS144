#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"
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
  if(!SYN && !message.SYN){
    return;
  }
  uint64_t abs_seqno = 0;
  if (message.SYN){
    SYN = true;
    ISN = message.seqno;
    abs_seqno = message.seqno.unwrap(ISN, checkpoint) + 1;
  }else{
    abs_seqno = message.seqno.unwrap(ISN, checkpoint);
  }
  string str = message.payload.release();
  cout << "payload: " << str << endl;
  cout << "abs seqno: " << abs_seqno << endl;
  reassembler.insert(abs_seqno-1, str, message.FIN, inbound_stream);
  checkpoint += str.size();
  cout << "check point: " << checkpoint << endl;
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  // (void)inbound_stream;
  TCPReceiverMessage msg;
  if (inbound_stream.available_capacity() > UINT16_MAX){
    msg.window_size = UINT16_MAX;
  }else{
    msg.window_size = inbound_stream.available_capacity();
  }
  if(!SYN){
    return msg;
  }
  uint64_t written = inbound_stream.bytes_pushed() + 1;
  if(inbound_stream.is_closed()){
    written++;
  }
  msg.ackno = Wrap32::wrap(written, ISN);
  return msg;
}
