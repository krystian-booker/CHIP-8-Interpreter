#include "Core.h"
#include <stdint.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <string>

Core::Core() {
}

Core::~Core() {
}

// Clear the memory, registers and screen
void Core::initialize() {
    pc = 0x200; //Program counter starts at 0x200
    opcode = 0; //Reset current opcode
    I = 0;      //Reset index register
    sp = 0;     //Reset stack pointer

    //Clear display
    //Clear stack
    //Clear registers V0-VF
    //Clear memory

    //Load font set
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    //Reset timers
}

// Copy the program into the memory
void Core::loadGame(const char *romName) {
    std::ifstream inFile(romName, std::ios::binary | std::ios::in);
    if (!inFile.is_open()) {
        std::cerr << "Problem opening file." << std::endl;
        exit(1);
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inFile), {});
    if (((int) buffer.size() - 1) + (int) pc < 4096) {
        std::copy(buffer.begin(), buffer.end(), std::begin(memory) + (int) pc);
    } else {
        std::cerr << "Invalid rom size." << std::endl;
        exit(1);
    }

    if (debug) {
        for (int i = 0; i < 4096; ++i) {
            std::cout << (int) memory[i] << " ";
        }
    }

    inFile.close();
}

// Emulate one cycle of the system
void Core::emulateCycle() {
    // Fetch Opcode:
    // To fetch the pc will specify the location.
    // As one opcode is two bytes long we will need to grab
    // two successive bytes and merge them to get the opcode.
    // 1010001000000000 |  // 0xA200
    //         11110000 =  // 0xF0 (0x00F0)
    // ------------------
    // 1010001011110000    // 0xA2F0

    //shift first byte left by 8 bits then bitwise OR second byte
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode Opcode:
    // Break the opcode down for a multi-level switch statement
    // 1010001011110000 0xA2F0
    // 1111000000000000 0xF000
    // ----------------
    // 1010000000000000 0xA000
    switch (opcode & 0xF000) {
        case 0x000:
            switch (opcode & 0x000F) {
                case 0x000: // 0x00E0: Clears the screen
                    //Execute opcode
                    break;
                case 0x00E: // 0x00EE: returns from subroutine
                    //Execute opcode
                    break;
                default:
                    unknownOpcode();
            }
            break;
        default:
            unknownOpcode();
    }

    //Execute Opcode
    //0xA2F0: we need to store 0x2F0 into the index register I
    //Only 12 bits are containing the value we need to store.
    //we use a bitwise ANd to get rid of the first four bits.
    //1010001011110000 & // 0xA2F0 (opcode)
    //0000111111111111 = // 0x0FFF
    //------------------
    //0000001011110000   // 0x02F0 (0x2F0)
    //I = opcode & 0x0FFF;
    //pc += 2; //bc every instruction is 2 bytes long, we need the to increment the pc by 2

    //Update timers
    //Timers count down at 60Hz, we need to implement something that slow down the emulation cycle
    //Execute 60 opcodes in one second
}

unsigned short Core::getX(){
    //ex. opcode = 0x6801;
    //0110100000000001
    //0000111100000000
    //----------------
    //100000000000 >> 8 (shift right 8 bits, which removes 8 zeros)
    //1000 (8 in decimal/hex)
    return (opcode & 0x0F00) >> 8;
}

unsigned short Core::getY(){
    //ex. opcode = 0x5560;
    //0101010101100000
    //0000000011110000
    //----------------
    //0000000001100000 >> 4 (shift right 4 bits, which removes 4 zeros)
    //1111 (15 in decimal/ F in hex)
    return (opcode & 0x00F0) >> 4;
}

unsigned short Core::getNN(){
    return (opcode & 0x00FF);
}

unsigned short Core::getNNN(){
    return (opcode & 0x0FFF);
}

void Core::unknownOpcode() {
    std::cerr << "Unknown opcode: " << opcode << std::endl;
    exit(1);
}