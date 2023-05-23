#include "network_interface.hh"

#include "address.hh"
#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "ethernet_header.hh"
#include "ipv4_datagram.hh"
#include "ipv4_header.hh"
#include "parser.hh"
#include <cstddef>
#include <cstdint>
#include <utility>

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address)
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t next_hop_ip = next_hop.ipv4_numeric();
  auto iter = arp_table.find(next_hop_ip);
  EthernetFrame frame;
  //if ip address of the next hop was not found in the arp table
  if (iter == arp_table.end()) {
    //add datagram to the wait_to_be_sent queue
    // wait_to_be_sent_dgram.push_back({next_hop, dgram});
    wait_to_be_sent_dgram.push_back({next_hop, dgram});
    //if no previous arp request has been broadcast before, broadcast it
    if(wait_response_msg.find(next_hop_ip) == wait_response_msg.end()){
      frame = broadcast_frame(next_hop_ip, {}, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
      //push the msg into the sent_msg queue and wait_for_response queue
      sent_frame.push(frame);
      wait_response_msg[next_hop_ip] = _timer;
    }
  } else {
    //if we already know the ip address of next hop
    //sent the frame and add the frame to the sent_frame_queue
    frame.header.type = EthernetHeader::TYPE_IPv4;
    frame.header.src = ethernet_address_;
    frame.header.dst = arp_table[next_hop_ip].first;
    frame.payload = serialize(dgram);
    sent_frame.push(frame);
  }
}
// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  EthernetHeader header = frame.header;
  optional<InternetDatagram> output;
  //if the destination ofthe frame was neither the broadcast address or the interface’s own Ethernet address
  //ignore
  if (!Ethernet_Address_equal(header.dst, ethernet_address_) && !Ethernet_Address_equal(header.dst, ETHERNET_BROADCAST)){
    return output;
  }
  //if the inbound frame is IPv4
  if(header.type == EthernetHeader::TYPE_IPv4){
    //parse the payload as an InternetDatagram
    InternetDatagram ip_datagram;
    //if sucessful, return the resulting InternetDatagram to the caller.
    if (parse(ip_datagram, frame.payload)){
      return ip_datagram;
    }
  }else{
    //if the inbound frame is ARP, parse the payload as an ARPMessag
    ARPMessage msg;
    if (parse(msg, frame.payload)){
      //if sucessful, map the sender’s IP address and Ethernet address for 30 seconds
      EthernetAddress eth_adr = msg.sender_ethernet_address;
      uint32_t ip_adr = msg.sender_ip_address;
      arp_table[ip_adr] = {eth_adr, _timer};
      wait_response_msg.erase(ip_adr);
      //send the pending msg in the queue use the updated arp table
      for (auto iter = wait_to_be_sent_dgram.begin(); iter != wait_to_be_sent_dgram.end();) {
        Address ip = iter->first;
        InternetDatagram dgram = iter->second;
        if (ip.ipv4_numeric() == ip_adr) {
          send_datagram(dgram, ip);
          wait_to_be_sent_dgram.erase(iter++);
        }else{
          iter++;
        }
      } 
      //if it’s an ARP request asking for our IP address, send an appropriate ARP reply
      if ((msg.opcode == ARPMessage::OPCODE_REQUEST) && (msg.target_ip_address == ip_address_.ipv4_numeric())){
        EthernetFrame frame_send = broadcast_frame(msg.sender_ip_address, msg.sender_ethernet_address, msg.sender_ethernet_address, ARPMessage::OPCODE_REPLY);
        sent_frame.push(frame_send);
      }  
    }
  }
  return output;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  _timer += ms_since_last_tick;

  //delete the ip that has been sent for more than 30s
  for (auto it = arp_table.begin(); it != arp_table.end();) {
    if (_timer - (it->second).second >= 30 * 1000) {
        arp_table.erase(it++);
    } else {
        it++;
    }
  }

  //re-broadcast the msg that has been wait for more than 5s
  for (auto & iter : wait_response_msg) {
    if (_timer - iter.second >= 5 * 1000) {
        EthernetFrame frame = broadcast_frame(iter.first, {}, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
        sent_frame.push(frame);
        iter.second = _timer;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  //actually send the maybe send from the queue
  optional<EthernetFrame> output;
  if (!sent_frame.empty()){
    output = sent_frame.front();
    sent_frame.pop();
    cout <<"not empty" << endl;
  }else{
    cout <<"empty" << endl;
  }
  return output;
}

//helper function to broadcast
EthernetFrame NetworkInterface::broadcast_frame(uint32_t ip, EthernetAddress target_ethernet_address, EthernetAddress dst_address, uint16_t opcode) {
  ARPMessage arp_msg;
  arp_msg.opcode = opcode;
  arp_msg.sender_ethernet_address = ethernet_address_;
  arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
  arp_msg.target_ethernet_address = target_ethernet_address;
  arp_msg.target_ip_address = ip;

  EthernetFrame frame;
  frame.header.src = ethernet_address_;
  frame.header.dst = dst_address;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.payload = serialize(arp_msg);

  return frame;
}

//helper function to compare mac address
bool NetworkInterface::Ethernet_Address_equal(EthernetAddress adr1, EthernetAddress adr2){
  for (int i = 0; i < 6; i++) {
    if (adr1[i] != adr2[i]) {
        return false;
    }
  }
  return true;
}
