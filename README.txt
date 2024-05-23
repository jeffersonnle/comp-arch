Jefferson Le
jnl9695@nyu.edu
proj2

I completed my project. I just spent a long time rewriting the structure of my cache function. simcache.cpp is a e20 simulator that is also able to simulate the use of caches, up to 2 levels. As far as I know, I've found all the bugs I can and have fixed them. 

Resources
Polytechnic Tutoring Center (for CS2214)
Recitation TAs

Design Decisions
Honestly, I should not have used structs. I think if I had simply used a vector of vector of vectors type data structure, the code would have been much easier, and easier to debug. Because I used structs, my cache function became very complex. Additionally, since I did decide to use structs, I should have used a helper function to move the repetitive parts of my cache function out. That would have made my code much cleaner, and would allow me to debug easier, because I kept changing portions while debugging, but forgetting to fix other portions. My original cache function structure was also honestly very overcomplicated and had unnecessary loops. You can see the original function in my comments, but I had one separate set of if statements for each of the following cases: L1 SW, L1 L2 SW, L1 LW, L1 L2 LW. This was extremely unnecessary and made it incredibly hard to debug. After I changed the structure to doing the L1 logic set regardless, and checking if L2 existed before moving to the L2 set, and not making it nested, it was much easier to debug, and I was able to find my earlier few mistakes in my L2 calculations.
