Jefferson Le
jnl9695@nyu.edu

proj1 - CS2214

I did complete project 1. It passes the 9 cases in the autograder, and I've created an E20 simulator that successfully runs. So far I've found no bugs in my code. The resources I used were primarily the E20 manual as well as the TA slides from recitations on E20 processors, from a couple weeks back. 

Design Decisions
The first time I wrote the simulator I actually defined every single regSrc, regDst, regA, regB, and regAddr directly inside each if statemement for each opcode. I thought that it made more sense that way, since depending on the instruction, regDst wouldn't be the same 3 bits as regDst in a different instruction (for example, regDst is bits 9, 8, and 7 in addi, but regDst is also bits 6, 5 and 4 in slt.) but it ended up being extremely messy, and it made the code very difficult to read. So I instead defined three ambiguous registers A, B and C at the start of every new instruction read. This made the instructions less intuitive to read when I was reading them side by side with the E20 manual, but it made the program as a whole much shorter. I also tried doing bitwise operations with entire binary lines in my first tries writing the project, and though this worked for most of the time, it was just really ugly to read and it was incredibly easy for me to make a mistake. So, I just shifted first and then did the & bitwise comparison, instead of doing it vice versa, where I compared it to a full 16 bits, then shifted the portion I wanted. Alsom, I put a line that sets register 0 to 0 at the end of every completed while loop. At first I put this line after every instruction that could possibly attempt to change register 0, but I thought it was more concise to put it once at the end. A strength of my program is probably the change I made to the register initialization, initializing them earlier all at once. Also, making the bit extension separate saves space.

Questions:
- What are the initial values of the registers, the program counter, and the memory cells?
Registers are all initially set to 0. The PC is set to 0. The memory cells are also set to 0.
– What should happen if a program sets a register to a value outside of the range expressible as a
16-bit unsigned number? Consider both positive and negative numbers that cannot be expressed
in 16 bits.
Then the number loops back around from the 16-bit maximum. It would overflow to 1, then 2, etc. etc.
– What should happen if a program tries to change the value of $0?
Nothing happens - aka, the register 0 will be set back to 0.
– What should happen if a program uses slt to compare a negative number to a positive number?
The signed representations will be compared, therefore the positive number will be larger.
– What range of memory address are valid? What should happen if a program tries to read or write
a memory cell whose address is outside of the range of valid addresses?
0-8191
– What should happen if a program sets the program counter to a value outside of the range of
valid addresses?
It will loop back around, basically it would do this: invalid_pc % 8191.
– What should happen if a program uses a negative immediate value in addi or jeq?
The program will change the negative number as a extremely large positive number.
– What should happen if a program uses a negative immediate value in lw or sw?
The program would sign-extend again.
– What should happen if a program modifies a memory cell containing machine code?
The program will crash.
– What should happen if the program counter reaches the address of the last memory cell?
It should pc+=1, and loop back around to the first instruction.
– When should your simulator stop?
At halt.