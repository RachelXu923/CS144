Checkpoint 2 Writeup
====================

My name: Zixuan Xu

My SUNet ID: zixuanxu

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: 
1. rqzhang
2. zherui
We discussed at the Tuesday's lab about how to handling the edge case in part one and their advice is really helpful!

This lab took me about 6 hours to do. I did attend the lab session.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
1. wrap/unwrap: obtain the abs seq based on the offset between checkpoint and X. The seqno offset should equal the absolute seqno. Then try to find the closest point by adding or minus (2^32) and compare their distance. 
2. TCP receiver: receive(): use a bool to store SYN flag; a Wrap32 to store ISN. First set SYN flag and ISN. Then obtain the first index using unwrap. send(): first determine the window size = min(UINT16_MAX, writer's availability). Then obtain the optional ackno based (SYN)+bytes pushed. Also include (FIN) if it is closed. 

Implementation Challenges:
[Edge case(int overflow, find the closest point) in wrap/unwrap took me longer time than expected.]

Remaining Bugs:
[None]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
