Checkpoint 4 Writeup
====================

My name: Zixuan Xu

My SUNet ID: zixuanxu

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about 6 hours to do. I did attend the lab session.

Program Structure and Design of the NetworkInterface:
I use an unordered_map (arp_table, <ip, <mac address, TTL>>) to store the known ip and its Ethernet Address. A queue of EthernetFrame (sent_frame) to store all EthernetFrame that has been sent. An unorded_map (wait_response_msg, <ip, TTL>) to store all the ARP request that is waiting for ARP response, including their ip and TTL. A list of dataframe (wait_to_be_sent_dgram, <ip, InternetDatagram>) wait to be sent as its Ethernet Address is unknown. 
In send_datagram function, if we already had the mac address of next_hop_ip in the arp_table, we directly send the datagram (add EthernetFrame to the sent_frame queue). If its mac address is unknown, add <ip, datagram> to wait_to_be_sent_dgram list. If no previous ARP request for this ip is found in wait_response_msg, broadcast the ARP request and add it to sent_frame and wait_response_msg.
In recv_datagram function, if it is an InternetDatagram, return. If it is an ARP request, add sender's ip and mac address to arp_table and erase the ip in wait_response_msg. Then send datagram in wait_to_be_sent_dgram whose target ip is sender's ip. Then reply the request if it is asking for our IP address (add arp reply to sent_frame).
In tick function, update the current timer. Then delete all the ip expired (>30s) and msg that has been wait for more than 5s. 
In maybe send function, send the first EthernetFrame if it is not empty. 

Implementation Challenges:
It took me quite a while to figure out what data structure I need to store all the information (ip, mac address, datagram, arp msg). 

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
