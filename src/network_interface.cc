#include "network_interface.hh"

#include "address.hh"
#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "ethernet_header.hh"
#include "ipv4_datagram.hh"
#include "ipv4_header.hh"
#include "parser.hh"
#include <cstdint>

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
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
  // (void)dgram;
  // (void)next_hop;
  const uint32_t next_hop_ip = next_hop.ipv4_numeric();
  auto iter = ARP_table.find(next_hop_ip);
  if (iter == ARP_table.end()) {
    waiting_to_be_sent.push_back({next_hop, dgram});
    if(waiting_response_msg.find(next_hop_ip) == waiting_response_msg.end()){
      EthernetFrame frame = broadcast_frame(next_hop_ip, {}, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
      frames_out_.push(frame);
      waiting_response_msg[next_hop_ip] = _timer;
    }
  } else {
    EthernetFrame frame;
    frame.header.type = EthernetHeader::TYPE_IPv4;
    frame.header.src = ethernet_address_;
    frame.payload = serialize(dgram);
    frame.header.dst = ARP_table[next_hop_ip].first;
    frames_out_.push(frame);
  }
  
}
// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // (void)frame;
  // return {};
  EthernetHeader header = frame.header;
  optional<InternetDatagram> output;
  if (!Ethernet_Address_equal(header.dst, ethernet_address_) && !Ethernet_Address_equal(header.dst, ETHERNET_BROADCAST)){
    return output;
  }
  if(header.type == EthernetHeader::TYPE_IPv4){
    InternetDatagram ip_datagram;
    bool res = parse(ip_datagram, frame.payload);
    if (res){
      return ip_datagram;
    }
  }else{
    ARPMessage msg;
    if (parse(msg, frame.payload)){
      EthernetAddress eth_adr = msg.sender_ethernet_address;
      uint32_t ip_adr = msg.sender_ip_address;
      if ((msg.opcode == ARPMessage::OPCODE_REQUEST) && (msg.target_ip_address == ip_address_.ipv4_numeric())){
        EthernetFrame frame_send = broadcast_frame(msg.sender_ip_address, msg.sender_ethernet_address, msg.sender_ethernet_address, ARPMessage::OPCODE_REPLY);
        frames_out_.push(frame_send);
      }
      ARP_table[ip_adr] = {eth_adr, _timer};
      for (auto iter = waiting_to_be_sent.begin(); iter != waiting_to_be_sent.end();) {
        Address addr_cache = iter->first;
        InternetDatagram dgram_cache = iter->second;
        if (addr_cache.ipv4_numeric() == ip_adr) {
            send_datagram(dgram_cache, addr_cache);
            waiting_to_be_sent.erase(iter++);
        } else {
            iter++;
        }
      }  
      waiting_response_msg.erase(ip_adr);    
    }
    // return output;
  }
  return output;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // (void)ms_since_last_tick;
  _timer += ms_since_last_tick;

  for (auto it = ARP_table.begin(); it != ARP_table.end();) {
    if (_timer - (it->second).second >= 30 * 1000) {
        ARP_table.erase(it++);
    } else {
        it++;
    }
  }

  for (auto & iter : waiting_response_msg) {
    if (_timer - iter.second >= 5 * 1000) {
        EthernetFrame frame = broadcast_frame(iter.first, {}, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
        frames_out_.push(frame);
        iter.second = _timer;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  optional<EthernetFrame> output;
  if (!frames_out_.empty()){
    output = frames_out_.front();
    frames_out_.pop();
  }
  return output;
}

EthernetFrame NetworkInterface::broadcast_frame(uint32_t ip, EthernetAddress target_ethernet_address, EthernetAddress dst_address, uint16_t opcode) {
  ARPMessage arp_msg;
  // arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
  arp_msg.opcode = opcode;
  arp_msg.sender_ethernet_address = ethernet_address_;
  arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
  arp_msg.target_ethernet_address = target_ethernet_address;
  arp_msg.target_ip_address = ip;

  EthernetFrame frame;
  frame.header.src = ethernet_address_;
  // frame.header.dst = ETHERNET_BROADCAST;
  frame.header.dst = dst_address;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.payload = serialize(arp_msg);

  return frame;
}

bool NetworkInterface::Ethernet_Address_equal(EthernetAddress adr1, EthernetAddress adr2){
  for (int i = 0; i < 6; i++) {
    if (adr1[i] != adr2[i]) {
        return false;
    }
  }
  return true;
}
