#include "router.hh"
#include "address.hh"
#include "ipv4_datagram.hh"

#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  routing_table.push_back(RouterItem{route_prefix, prefix_length, next_hop, interface_num});
}

void Router::route() {
  for (auto & interface : interfaces_) {
    auto maybe_datagram = interface.maybe_receive();
    while (maybe_datagram.has_value()) {
      InternetDatagram& datagram = maybe_datagram.value();
      auto best_route = routing_table.end();
      for(auto iter = routing_table.begin(); iter != routing_table.end();
            iter++){
        if (prefix_equal(datagram.header.dst, iter->route_prefix, iter->prefix_length)) {
          if (best_route == routing_table.end() || iter->prefix_length > best_route->prefix_length) {
            best_route = iter;
          }
        }
      }

      if (best_route != routing_table.end() && datagram.header.ttl-- > 1) {
        datagram.header.compute_checksum();
        const size_t interface_num = best_route->interface_num;
        optional<Address> next_hop = best_route->next_hop;
        if (next_hop.has_value()) {
          interfaces_[interface_num].send_datagram(datagram, next_hop.value());
        } else {
          Address dst = Address::from_ipv4_numeric(datagram.header.dst);
          interfaces_[interface_num].send_datagram(datagram, dst);
        }
      }
      maybe_datagram = interface.maybe_receive();
    }
  }
}

bool Router::prefix_equal(uint32_t dst_ip, uint32_t route_prefix, uint8_t prefix_length){
  // uint32_t mask = (prefix_length == 0) ? 0 : (0xffffffff << (32 - prefix_length));
  // return (dst_ip & mask) == (route_prefix & mask);
  // uint8_t bits_to_compare = prefix_length;

  // Compare the bits up to the specified prefix length
  // for (uint8_t i = 0; i < prefix_length; ++i) {
  //   uint32_t dst_bit = (dst_ip >> (31 - i));
  //   uint32_t prefix_bit = (route_prefix >> (31 - i));
  //   if (dst_bit != prefix_bit) {
  //     return false;
  //   }
  // }
  // return true;
  if (prefix_length == 0){
    return true;
  }
  uint32_t ip1 = dst_ip >> (32 - prefix_length);
  uint32_t ip2 = route_prefix >> (32 - prefix_length);
  return ip1 == ip2;
}
