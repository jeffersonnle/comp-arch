/*
CS-UY 2214
Adapted from Jeff Epstein
Starter code for E20 cache Simulator
simcache.cpp
*/

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <fstream>
#include <limits>
#include <iomanip>

#include <regex>
#include <cstdlib>


using namespace std; 
// Some helpful constant values that we'll be using.
size_t const static NUM_REGS = 8;
size_t const static MEM_SIZE = 1<<13;
size_t const static REG_SIZE = 1<<16;

// PROJ 1 IMPORTED CODE -------------------------------------------------------------------------------------------------------------
void extender(uint16_t& imm7) {
    if((imm7 >> 6) == 1) { imm7 = imm7|65408; }
}
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

// ----------------------------------------------------------------------------------------------------------------------------------

/*
    Prints out the correctly-formatted configuration of a cache.

    @param cache_name The name of the cache. "L1" or "L2"

    @param size The total size of the cache, measured in memory cells.
        Excludes metadata

    @param assoc The associativity of the cache. One of [1,2,4,8,16]

    @param blocksize The blocksize of the cache. One of [1,2,4,8,16,32,64])

    @param num_rows The number of rows in the given cache.
*/
void print_cache_config(const string &cache_name, int size, int assoc, int blocksize, int num_rows) {
    cout << "Cache " << cache_name << " has size " << size <<
        ", associativity " << assoc << ", blocksize " << blocksize <<
        ", rows " << num_rows << endl;
}

/*
    Prints out a correctly-formatted log entry.

    @param cache_name The name of the cache where the event
        occurred. "L1" or "L2"

    @param status The kind of cache event. "SW", "HIT", or
        "MISS"

    @param pc The program counter of the memory
        access instruction

    @param addr The memory address being accessed.

    @param row The cache row or set number where the data
        is stored.
*/
void print_log_entry(const string &cache_name, const string &status, int pc, int addr, int row) {
    cout << left << setw(8) << cache_name + " " + status <<  right <<
        " pc:" << setw(5) << pc <<
        "\taddr:" << setw(5) << addr <<
        "\trow:" << setw(4) << row << endl;
}

struct Block {
    int tag = -1;
};

struct Row {
    int assoc;
    vector<Block> blocks;
    Row(int assoc) : assoc(assoc) {
        for (int i = 0; i < assoc; i++) {
            blocks.push_back(Block{});
        }
    }
};

struct Level {
    vector<Row> rows;
    int assoc;
    int size;
    int blocksize;
    int num_rows;
    int level_size = 0;
    Level(int assoc, int size, int blocksize) : assoc(assoc), size(size), blocksize(blocksize), level_size(size/(assoc*blocksize)) {
        num_rows = (size/(assoc * blocksize));
        for (int i = 0; i < num_rows; i++) { 
            rows.push_back(Row(assoc));
        }
    }
};

struct Cache {
    vector<Level> levels;
};

// /*
void cache_func(Cache& cache_struct, uint16_t pc, uint16_t mem, bool op) {
    int num_rows = cache_struct.levels[0].num_rows;
    // Initialize L1 Values
    uint16_t blockID = mem/cache_struct.levels[0].blocksize;
    uint16_t row = blockID % cache_struct.levels[0].level_size;
    uint16_t tagID = blockID / cache_struct.levels[0].level_size;
    //uint16_t tag;
    bool hit = false;
    bool full = false;


    // If sw and L1 only
    if (op == true) { // if op true, then its a sw. at least L1.
        print_log_entry("L1", "SW", pc, mem, row);
        // remove the first block in vec (this is the "oldest" thing in the row)
        cache_struct.levels[0].rows[row].blocks.erase(cache_struct.levels[0].rows[row].blocks.begin());
        // store new data in the end of the vec (most recent spot in the row)
        cache_struct.levels[0].rows[row].blocks.push_back(Block{tagID});
        //return;
    } else if (op == false) { // if op false, its a lw. at least L1.
        for (int i = 0; i < cache_struct.levels[0].rows[row].blocks.size(); i++) {
            if (cache_struct.levels[0].rows[row].blocks[i].tag == tagID) { // hit, all good
                // remove the current instance of the data in the blocks vec
                cache_struct.levels[0].rows[row].blocks.erase(cache_struct.levels[0].rows[row].blocks.begin()+i);
                // repush the new data into the back of the vec, the most recent slot
                cache_struct.levels[0].rows[row].blocks.push_back(Block{tagID});
                print_log_entry("L1", "HIT", pc, mem, row);
                hit = true;
                break;
            } 
        }
             if (hit == false) { // miss L1, fetch from mem
                // swap to most recent tag, and evict the oldest tag CHECK, U SHOULD ONLY EVICT HERE IF IT IS FULL.
                // IF IT ISNT, JUST PUSH_BACK

                cache_struct.levels[0].rows[row].blocks.erase(cache_struct.levels[0].rows[row].blocks.begin()); 
                cache_struct.levels[0].rows[row].blocks.push_back(Block{tagID});
                print_log_entry("L1", "MISS", pc, mem, row);
                //hit = false;
            } 
    }
    

    // Now, if there's an L2: 
    if (cache_struct.levels.size() == 2 && hit == false) { // if there's an L2 and L1 missed OR is a SW, go here
        // now check if the previous operation was a sw, or a miss
        // if it was a sw we must do the L2 sw as well. if it was a miss, we must check L2 and see if L2 
        // hits or misses. if it was a hit in L1, we can ignore L2, since nothing will happen.
        // Initialize L2 Values
        int num_rows2 = cache_struct.levels[1].num_rows;
        uint16_t blockID2 = mem/cache_struct.levels[1].blocksize;
        uint16_t row2 = blockID2 % cache_struct.levels[1].level_size;
        uint16_t tagID2 = blockID2 / cache_struct.levels[1].level_size;

        if (op == true) { // if it was a sw, we must now also store in L2
            // remove the current first block in vec (this is the "oldest" thing in the row)
            cache_struct.levels[1].rows[row2].blocks.erase(cache_struct.levels[1].rows[row2].blocks.begin());
            // store new data in the end of the vec L2 (most recent spot in the row)
            cache_struct.levels[1].rows[row2].blocks.push_back(Block{tagID2});
            print_log_entry("L2", "SW", pc, mem, row2);
        } else { // if it was a lw, and L1 missed, we will check if L2 hits or misses
            for (int j = 0; j < cache_struct.levels[1].rows[row2].blocks.size(); j++) {
                //cout << cache_struct.levels[1].rows[row2].blocks[j].tag;
                //cout << tagID2;
                if (cache_struct.levels[1].rows[row2].blocks[j].tag == tagID2) { // hit L2
                    // remove the current instance of the data in the blocks vec
                    cache_struct.levels[1].rows[row2].blocks.erase(cache_struct.levels[1].rows[row2].blocks.begin()+j);
                    // repush the new data into the back of the vec, the most recent slot
                    cache_struct.levels[1].rows[row2].blocks.push_back(Block{tagID2});
                    print_log_entry("L2", "HIT", pc, mem, row2);
                    hit = true;
                    break;
                }
            }
            if (hit == false) { // miss L2, fetch from mem
                // swap to most recent tag, and evict the oldest tag
                cache_struct.levels[1].rows[row2].blocks.erase(cache_struct.levels[1].rows[row2].blocks.begin()); 
                cache_struct.levels[1].rows[row2].blocks.push_back(Block{tagID2});
                print_log_entry("L2", "MISS", pc, mem, row2);
                //hit = false;
            } 
        }
    }

}




// */

// first try
 /*
void cache_func(Cache& cache_struct, uint16_t pc, uint16_t mem, bool op) {  //sw and lw utilize this
    int num_rows = cache_struct.levels[0].rows.size();
    uint16_t blockID = mem/cache_struct.levels[0].blocksize;
    uint16_t row = blockID % cache_struct.levels[0].level_size;
    uint16_t tagID = blockID / cache_struct.levels[0].level_size;
    uint16_t tag;
    bool full = false;
    if (cache_struct.levels.size() == 1) {
        print_cache_config("L1", cache_struct.levels[0].size, cache_struct.levels[0].assoc, cache_struct.levels[0].blocksize, num_rows);
        // search through the row selected, look at all its blocks and see if any are hits
        for (int i = 0; i < cache_struct.levels[0].rows[row].blocks.size(); i++) {
            if (op == true) { // if SW & L1
                print_log_entry("L1", "SW", pc, mem, row);
            } else if (cache_struct.levels[0].rows[row].blocks[i].tag == blockID) { // hit, all good
                // swap to most recent tag, and evict the oldest tag
                
                cache_struct.levels[0].rows[row].blocks.erase(cache_struct.levels[0].rows[row].blocks.begin()+i);
                //Block temp;
                //temp.tag = tagID; 
                cache_struct.levels[0].rows[row].blocks.push_back(Block{tagID});
                print_log_entry("L1", "HIT", pc, mem, row);

            } else if (cache_struct.levels[0].rows[row].blocks[i].tag == -1) { // miss, fetch from mem
                cache_struct.levels[0].rows[row].blocks[i].tag = tagID;
                print_log_entry("L1", "MISS", pc, mem, row);
            } 
        }
    } else if (cache_struct.levels.size() == 2) { // L1 and L2
        // Level 2 Values
        int num_rows2 = cache_struct.levels[1].rows.size();
        uint16_t blockID2 = mem/cache_struct.levels[1].blocksize;
        uint16_t row2 = blockID2 % num_rows2;
        uint16_t tagID2 = uint16_t(blockID2 / num_rows2);
        uint16_t tag2;
        print_cache_config("L1", cache_struct.levels[0].size, cache_struct.levels[0].assoc, cache_struct.levels[0].blocksize, num_rows);
        print_cache_config("L2", cache_struct.levels[1].size, cache_struct.levels[1].assoc, cache_struct.levels[1].blocksize, num_rows2);

        for (int i = 0; i < cache_struct.levels[0].rows[row].blocks.size(); i++) {
            if (op == true) { // if SW & L1 & L2
                print_log_entry("L1", "SW", pc, mem, row);
                print_log_entry("L2", "SW", pc, mem, row2);
            } else if (cache_struct.levels[0].rows[row].blocks[i].tag == blockID) { // hit, all good
                // swap to most recent tag, and evict the oldest tag
                cache_struct.levels[0].rows[row].blocks.erase(cache_struct.levels[0].rows[row].blocks.begin()+i);
                cache_struct.levels[0].rows[row].blocks.push_back(Block{tagID});
                print_log_entry("L1", "HIT", pc, mem, row);

            } else if (cache_struct.levels[0].rows[row].blocks[i].tag == -1) { // miss + block is empty, fetch from mem
                cache_struct.levels[0].rows[row].blocks[i].tag = tagID;
                for (int i = 0; i < cache_struct.levels[1].rows[row2].blocks.size(); i++) {
                    if (cache_struct.levels[1].rows[row2].blocks[i].tag == blockID2) { // hit, all good
                        // swap to most recent tag, and evict the oldest tag
                        cache_struct.levels[1].rows[row2].blocks.erase(cache_struct.levels[0].rows[row2].blocks.begin()+i);
                        //Block temp;
                        //temp.tag = tagID; 
                        cache_struct.levels[1].rows[row2].blocks.push_back(Block{tagID2});
                        print_log_entry("L2", "HIT", pc, mem, row2);

                    } else if (cache_struct.levels[1].rows[row2].blocks[i].tag == -1) { // miss, fetch from mem
                        cache_struct.levels[1].rows[row2].blocks[i].tag = tagID2;
                        print_log_entry("L1", "MISS", pc, mem, row);
                        print_log_entry("L2", "MISS", pc, mem, row2);
                        }
                    }
                } 
        } 
    }

}
 */

void e20sim(uint16_t mem[], uint16_t regs[], uint16_t pc, Cache &cache_struct) {
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
            } else if (opcode == 4) { //100, lw // CACHEING HERE
                // regA is regAddr, regB is regDst, imm7 is imm
                // CACHE CALL HERE
                
                regs[regB] = mem[(imm7 + regs[regA]) % MEM_SIZE]; 
                //regs[0] = 0;
                pc += 1; 
                cache_func(cache_struct, pc2, (imm7 + regs[regA]) % MEM_SIZE, false); //CHECK
            } else if (opcode == 5) { //101, sw // CACHEING HERE
                // regA is regAddr, regB is regDst, imm7 is imm
                // CACHE CALL HERE
                
                mem[(imm7 + regs[regA]) % MEM_SIZE] = regs[regB];
                pc += 1;
                cache_func(cache_struct, pc2, (imm7 + regs[regA]) % MEM_SIZE, true); //CHECK
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

    // Imported Code From Proj1 Above
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
    string cache_config;
    for (int i=1; i<argc; i++) {
        string arg(argv[i]);
        if (arg.rfind("-",0)==0) {
            if (arg== "-h" || arg == "--help")
                do_help = true;
            else if (arg=="--cache") {
                i++;
                if (i>=argc)
                    arg_error = true;
                else
                    cache_config = argv[i];
            }
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
        cerr << "usage " << argv[0] << " [-h] [--cache CACHE] filename" << endl << endl;
        cerr << "Simulate E20 cache" << endl << endl;
        cerr << "positional arguments:" << endl;
        cerr << "  filename    The file containing machine code, typically with .bin suffix" << endl<<endl;
        cerr << "optional arguments:"<<endl;
        cerr << "  -h, --help  show this help message and exit"<<endl;
        cerr << "  --cache CACHE  Cache configuration: size,associativity,blocksize (for one"<<endl;
        cerr << "                 cache) or"<<endl;
        cerr << "                 size,associativity,blocksize,size,associativity,blocksize"<<endl;
        cerr << "                 (for two caches)"<<endl;
        return 1;
    }

    // FROM PROJ1 BELOW----------------------------------------------------------------------
        ifstream f(filename);
        if (!f.is_open()) {
            cerr << "Can't open file "<<filename<<endl;
            return 1;
        }
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
    // FROM PROJ1 ABOVE----------------------------------------------------------------------

    /* parse cache config */
    if (cache_config.size() > 0) {
        vector<int> parts;
        size_t pos;
        size_t lastpos = 0;
        while ((pos = cache_config.find(",", lastpos)) != string::npos) {
            parts.push_back(stoi(cache_config.substr(lastpos,pos)));
            lastpos = pos + 1;
        }
        parts.push_back(stoi(cache_config.substr(lastpos)));
        if (parts.size() == 3) {
            int L1size = parts[0];
            int L1assoc = parts[1];
            int L1blocksize = parts[2];
            // TODO: execute E20 program and simulate one cache here
            int num_rows = (L1size/(L1assoc * L1blocksize));
            Cache cache1;
            print_cache_config("L1", L1size, L1assoc, L1blocksize, num_rows);
            cache1.levels.push_back(Level(L1assoc, L1size, L1blocksize));
            e20sim(mem, regs, pc, cache1);
            
        } else if (parts.size() == 6) {
            int L1size = parts[0];
            int L1assoc = parts[1];
            int L1blocksize = parts[2];
            int L2size = parts[3];
            int L2assoc = parts[4];
            int L2blocksize = parts[5];
            // TODO: execute E20 program and simulate two caches here
            int num_rows1 = (L1size/(L1assoc * L1blocksize));
            int num_rows2 = (L2size/(L2assoc * L2blocksize));
            Cache cache1;
            print_cache_config("L1", L1size, L1assoc, L1blocksize, num_rows1);
            print_cache_config("L2", L2size, L2assoc, L2blocksize, num_rows2);
            cache1.levels.push_back(Level(L1assoc, L1size, L1blocksize));
            cache1.levels.push_back(Level(L2assoc, L2size, L2blocksize));
            e20sim(mem, regs, pc, cache1);

        } else {
            cerr << "Invalid cache config"  << endl;
            return 1;
        }
    }

    return 0;
}
//ra0Eequ6ucie6Jei0koh6phishohm9
