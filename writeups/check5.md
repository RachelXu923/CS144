Checkpoint 5 Writeup
====================

My name: Zixuan Xu

My SUNet ID: zixuanxu

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about 2 hours to do. I did attend the lab session.

Program Structure and Design of the Router:
I use a struct to store all the information of a route and then use a list to store all the route as a routing table. In add_route function, add a new route to the routing table. In route function, iterate all interfaces and all datagrames to find the longest-prefix-match route for each datagram. Then send each datagram to next hop or its destination. The helper function prefix_equal compare two route for prefix length. As we only care about most-significant prefix length bits of the destination address, shift two route by (32-prefix_length) then compare if they are equal. 

Implementation Challenges:
The most challenging part for me is to understand the route function and how to do the longest-prefix-match in a straight-forward way.

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: I spend half an hour dealing with bad payload problem as I forgot to compute the checksum after I decrement the ttl. 

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
