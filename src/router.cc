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

  //add a new route to the route table
  routing_table.push_back(Route{route_prefix, prefix_length, next_hop, interface_num});
}

void Router::route() {
  //route all datagrams from all the interfaces
  for (auto & interface : interfaces_) {
    auto maybe_datagram = interface.maybe_receive();
    //route the datagram as long as interface is not empty
    while (maybe_datagram.has_value()) {
      InternetDatagram& datagram = maybe_datagram.value();
      //Initiate the best route
      auto best_route = routing_table.end();
      //search the routing table
      for(auto iter = routing_table.begin(); iter != routing_table.end();
            iter++){
        //if prefix of the destination ip and route equal, we found a candidate route
        if (prefix_equal(datagram.header.dst, iter->route_prefix, iter->prefix_length)) {
          //update the best_route if no best_route has been found, or the prefix length of new route is longer
          if (best_route == routing_table.end() || iter->prefix_length > best_route->prefix_length) {
            best_route = iter;
          }
        }
      }
      //if we found a route and ttl is larger than 1(so that ttl > 0 after update)
      if (best_route != routing_table.end() && datagram.header.ttl-- > 1) {
        //!!!recalculate the checksum after decrementing the TTL
        datagram.header.compute_checksum();
        const size_t interface_num = best_route->interface_num;
        optional<Address> next_hop = best_route->next_hop;
        //if next hop has value, send the datagram to the next hop
        if (next_hop.has_value()) {
          interfaces_[interface_num].send_datagram(datagram, next_hop.value());
        } else {
          //or send the datagram to its destination
          Address dst = Address::from_ipv4_numeric(datagram.header.dst);
          interfaces_[interface_num].send_datagram(datagram, dst);
        }
      }
      maybe_datagram = interface.maybe_receive();
    }
  }
}

bool Router::prefix_equal(uint32_t dst_ip, uint32_t route_prefix, uint8_t prefix_length){
  //if prefix length is zero, return true as always equal
  if (prefix_length == 0){
    return true;
  }
  //if prefix length is not zero
  //right shif dst ip and route by (32 - prefix_length) positions
  //return true is they are same
  uint32_t ip1 = dst_ip >> (32 - prefix_length);
  uint32_t ip2 = route_prefix >> (32 - prefix_length);
  return ip1 == ip2;
}
