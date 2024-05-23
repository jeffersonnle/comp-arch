/*
CS-UY 2214
Adapted from Jeff Epstein
Starter code for E20 simulator
sim.cpp
*/

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <regex>
#include <cstdlib>
//#include <ctsdint>

using namespace std;

// Some helpful constant values that we'll be using.
size_t const static NUM_REGS = 8;
size_t const static MEM_SIZE = 1<<13;
size_t const static REG_SIZE = 1<<16;

/*
    Loads an E20 machine code file into the list
    provided by mem. We assume that mem is
    large enough to hold the values in the machine
    code file.

    @param f Open file to read from
    @param mem Array represetnting memory into which to read program
*/
void load_machine_code(ifstream &f, uint16_t mem[]) {
    regex machine_code_re("^ram\\[(\\d+)\\] = 16'b(\\d+);.*$");
    size_t expectedaddr = 0;
    string line;
    while (getline(f, line)) {
        smatch sm;
        if (!regex_match(line, sm, machine_code_re)) {
            cerr << "Can't parse line: " << line << endl;
            exit(1);
        }
        size_t addr = stoi(sm[1], nullptr, 10);
        unsigned instr = stoi(sm[2], nullptr, 2);
        if (addr != expectedaddr) {
            cerr << "Memory addresses encountered out of sequence: " << addr << endl;
            exit(1);
        }
        if (addr >= MEM_SIZE) {
            cerr << "Program too big for memory" << endl;
            exit(1);
        }
        expectedaddr ++;
        mem[addr] = instr;
    }
}

/*
    Prints the current state of the simulator, including
    the current program counter, the current register values,
    and the first memquantity elements of memory.

    @param pc The final value of the program counter
    @param regs Final value of all registers
    @param memory Final value of memory
    @param memquantity How many words of memory to dump
*/
void print_state(uint16_t pc, uint16_t regs[], uint16_t memory[], size_t memquantity) {
    cout << setfill(' ');
    cout << "Final state:" << endl;
    cout << "\tpc=" <<setw(5)<< pc << endl;

    for (size_t reg=0; reg<NUM_REGS; reg++)
        cout << "\t$" << reg << "="<<setw(5)<<regs[reg]<<endl;

    cout << setfill('0');
    bool cr = false;
    for (size_t count=0; count<memquantity; count++) {
        cout << hex << setw(4) << memory[count] << " ";
        cr = true;
        if (count % 8 == 7) {
            cout << endl;
            cr = false;
        }
    }
    if (cr)
        cout << endl;
}

// simulator accepts a dynamic array of the machine code instructions read by 
// load_machine_code, and simulates an E20 processor running through them

void extender(uint16_t& imm7) {
    if((imm7 >> 6) == 1) { imm7 = imm7|65408; }
}

/**
    Main function
    Takes command-line args as documented below
*/
int main(int argc, char *argv[]) {
    /*
        Parse the command-line arguments
    */
    char *filename = nullptr;
    bool do_help = false;
    bool arg_error = false;
    for (int i=1; i<argc; i++) {
        string arg(argv[i]);
        if (arg.rfind("-",0)==0) {
            if (arg== "-h" || arg == "--help")
                do_help = true;
            else
                arg_error = true;
        } else {
            if (filename == nullptr)
                filename = argv[i];
            else
                arg_error = true;
        }
    }
    /* Display error message if appropriate */
    if (arg_error || do_help || filename == nullptr) {
        cerr << "usage " << argv[0] << " [-h] filename" << endl << endl;
        cerr << "Simulate E20 machine" << endl << endl;
        cerr << "positional arguments:" << endl;
        cerr << "  filename    The file containing machine code, typically with .bin suffix" << endl<<endl;
        cerr << "optional arguments:"<<endl;
        cerr << "  -h, --help  show this help message and exit"<<endl;
        return 1;
    }

    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Can't open file "<<filename<<endl;
        return 1;
    }
    // TODO: your code here. Load f and parse using load_machine_code
    uint16_t pc = 0;
    uint16_t regs[NUM_REGS];
    uint16_t mem[MEM_SIZE];
    //set defaults to 0
    for (size_t i = 0; i < NUM_REGS; i++) {
        regs[i] = 0;
    }
    for (size_t i = 0; i < MEM_SIZE; i++) {
        mem[i] = 0;
    }
    load_machine_code(f, mem);
    bool halt = false;

    // TODO: your code here. Do simulation.

    while (halt == false) { 
   
        uint16_t pc2 = pc % MEM_SIZE; // PC compensated for overflow
        uint16_t opcode = mem[pc2] >> 13; //opcode
        uint16_t regA = (mem[pc2] >> 10) & 7; //2nd 3 bits, first reg
        uint16_t regB = (mem[pc2] >> 7) & 7; //3rd 3 bits, second reg
        uint16_t regC = (mem[pc2] >> 4) & 7; //4th 3 bits, third reg

        // Immediate Values Below
        uint16_t imm4 = mem[pc2] & 15;
        uint16_t imm7 = mem[pc2] & 127;
        extender(imm7);
        uint16_t imm13 = mem[pc2] & 8191;
    

        if (opcode == 0) //000, add/sub/or/and/slt/jr
        {
            if (imm4 == 0) { //0000, add OK
                regs[regC] = regs[regA] + regs[regB];
                pc += 1;

            } else if (imm4 == 1) { //0001, sub OK
                regs[regC] = regs[regA] - regs[regB];
                pc += 1;

            } else if (imm4 == 2) { //0010, or OK
                regs[regC] = regs[regA] | regs[regB];
                pc += 1;

            } else if (imm4 == 3) { //0011, and OK
                regs[regC] = regs[regA] & regs[regB];
                pc += 1;

            } else if (imm4 == 4) { //0100, slt OK
                if (regs[regA] < regs[regB]) {regs[regC] = 1;} else {regs[regC] = 0;}
                pc += 1;

            } else if (imm4 == 8) { //1000, jr OK
                pc = regs[regA]; 
                //regs[0] = 0;
            }

        } else if (opcode == 7) { //111, slti OK
            // regA is regSrc, regB is regDst, imm7 is imm
            if (imm7 > regs[regA]) { regs[regB] = 1;} else { regs[regB] = 0;}
            //regs[0] = 0;
            pc += 1;
        } else if (opcode == 4) { //100, lw
            // regA is regAddr, regB is regDst, imm7 is imm
            regs[regB] = mem[(imm7 + regs[regA]) % MEM_SIZE]; 
            //regs[0] = 0;
            pc += 1; 
        } else if (opcode == 5) { //101, sw
            // regA is regAddr, regB is regDst, imm7 is imm
            mem[(imm7 + regs[regA]) % MEM_SIZE] = regs[regB];
            pc += 1;
        } else if (opcode == 6) { //110, jeq
            if (regs[regA] == regs[regB]) { pc += imm7 + 1; } else { pc += 1; }
          
        } else if (opcode == 1) { //001, addi
           // regA is regSrc, regB is regDst, imm7 is imm
            regs[regB] = regs[regA] + imm7;
            //regs[0] = 0;
            pc += 1;
            
        } else if (opcode == 2) { //010, j imm
            if (pc == imm13) { halt = true; } else { pc = imm13; }
            
        } else if (opcode == 3) { //011, jal imm
            regs[7] = pc+1;
            pc = imm13;
        } 
        regs[0] = 0;

    }

    // TODO: your code here. print the final state of the simulator before ending, using print_state
    print_state(pc, regs, mem, 128);
    return 0;
}
//ra0Eequ6ucie6Jei0koh6phishohm9
