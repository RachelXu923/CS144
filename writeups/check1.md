Checkpoint 1 Writeup
====================

My name: Zixuan Xu

My SUNet ID: zixuanxu

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: 
1. rqzhang
2. cuizk
We discussed on Tuesday's lab about data structure (Map vs. Vecto vs. LinkedList) and high-level approach that will have better performance.

This lab took me about 6 hours to do. I did attend the lab session.

Program Structure and Design of the Reassembler:
I used map<first index, data> to store the data blocks I received and haven't writting into the buffer:
- All the data blocks in the data are not overlapped. 
- Every time when I received a piece of data, I will merge it into the map. 
- After merging, I will check the map from the begining to see if there's any data block that can be pushed into the writer until I found a gap between two block. 
- The Writer will be closed only when the last string has been received and map is empty (all the data has been pushed into the buffer). 

Implementation Challenges:
Merging data blocks is much more complicated than I expected. I basically discuss three merging cases: 
- Merging in the beginning
- Merging in the middle
- Merging in the end. 
To make sure there's no overlap in the map, I truncate the new data or delete the old data before adding. 

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
