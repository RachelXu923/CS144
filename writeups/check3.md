Checkpoint 3 Writeup
====================

My name: Zixuan Xu

My SUNet ID: zixuanxu

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: 
1. rqzhang
2. zherui
We discussed at and after the Tuesday's lab. I have a lot confusion about this week's handout and discussion with them enable me to be more clear about the definition. 

This checkpoint took me about 16 hours to do. I did attend the lab session.

Program Structure and Design of the TCPSender:
I used two deque to store the segments: one is pending_seg queue that stores all the msg that has been read but not sent yet. Another one is outstanding queue that stores all the segments that has been sent but not acked yet. In the push function, read as much bytes as possbile and push them into the pending queue. In maybe_send function, send the first msg in the pending queue(pop it) and push it into the outstanding queue. When received ack msg, drop all segments that has been acked in the outstanding queue. In tick function, update the timer current time. If expired, retransmit the msg by remove segment from the outstanding queue and add it to the pending queue (so that it could be resent in maybe_send). 

Implementation Challenges:
I spent hours reading the spec and understanding the maybe_send function, try to figure out where the actual send happens. 

Remaining Bugs:
[None]

- Optional: I had unexpected difficulty with: Understanding the maybe_send function and determine when to update the two queue I used. 

- Optional: I think you could make this lab better by: Maybe more explanation and details about the definition of the functions. 

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
