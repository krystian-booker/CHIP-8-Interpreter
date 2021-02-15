#include "Core.h"
#include <fstream>
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
    for (int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }

    //Clear stack
    stack[16] = {0};

    //Clear registers V0-VF
    V[16] = {0};

    //Clear memory
    memory[4096] = {0};

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
        case 0x0000: {
            // 11100000 0x00E0
            // 00001111 0x000F
            // ----------------
            // 00000000 0x000
            switch (opcode & 0x000F) {
                case 0x000: {// 0x00E0: Clears the screen
                    for (int i = 0; i < 2048; ++i) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
                    pc += 2;
                }
                    break;
                case 0x00E: {// 0x00EE: returns from subroutine
                    if (sp == 0) {
                        std::cerr << "Stack pointer out of range" << std::endl;
                        exit(1);
                    }
                    pc = stack[--sp];
                }
                    break;
                default: {
                    unknownOpcode();
                }
            }
        }
            break;
        case 0x1000: {// 1NNN: Jumps to address NNN
            pc = getNNN();
        }
            break;
        case 0x2000: { // 2NNN: Calls subroutine at NNN
            //Store current program counter location
            stack[sp++] = pc + 2;
            pc = getNNN();
        }
            break;
        case 0x3000: {// 3XNN: Skips the next instruction if VX equals NN
            if (V[getX()] == getNN()) {
                pc += 2;
            }
            pc += 2;
        }
            break;
        case 0x4000: {// 4XNN: Skips the next instruction if VX doesn't equal NN
            if (V[getX()] != getNN()) {
                pc += 2;
            }
            pc += 2;
        }
            break;
        case 0x5000: {// 5XY0: Skips the next instruction if VX equals VY
            if (V[getX()] == V[getY()]) {
                pc += 2;
            }
            pc += 2;
        }
            break;
        case 0x6000: {// 6XNN: Sets VX to NN
            V[getX()] = getNN();
            pc += 2;
        }
            break;
        case 0x7000: {// 7XNN: Adds NN to VX. (Carry flag is not changed)
            V[getX()] += getNN();
            pc += 2;
        }
            break;
        case 0x8000: {
            //TODO: Execute opcode
            unknownOpcode();
        }
            break;
        case 0x9000: {// 9XY0: Skips the next instruction if VX doesn't equal VY
            if (V[getX()] != V[getY()]) {
                pc += 2;
            }
            pc += 2;
        }
            break;
        case 0xA000: {// ANNN: Sets I to the address NNN
            I = getNNN();
            pc += 2;
        }
            break;
        case 0xD000: {// DXYN: Draws a sprite at coordinate (VX, VY)
            unsigned short xPos = V[getX()];
            unsigned short yPos = V[getY()];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0; //Clear collisions
            for (int yLine = 0; yLine < height; yLine++) { //Loop through height of sprite
                pixel = memory[I + yLine]; //grab sprite pixel row
                for (int xLine = 0; xLine < 8; xLine++) { //loop through all 8 bits in pixel row
                    if ((pixel & (0x80 >> xLine)) != 0) { //check if current pixel is set to 1
                        if (gfx[(xPos + xLine + ((yPos + yLine) * 64))] == 1) {//register collision if pixel is 1
                            V[0xF] = 1;
                        }
                    }
                    gfx[xPos + xLine + ((yPos + yLine) * 64)] ^= 1; //XOR the pixel value
                }
            }

            drawFlag = true;
            pc += 2;
        }
            break;
        case 0xF000: {
            // 1111000000101001 0xF029
            // 0000000011111111 0x00FF
            // ----------------
            // 0000000000101001 0x029
            switch (opcode & 0x00FF) {
                case 0x00A: {// FX0A: A key press is awaited, and then stored in VX.
                    // (Blocking Operation. All instruction halted until next key event)
                    V[getX()] = getKey();
                    pc += 2;
                }
                    break;
                case 0x01E: {// FX1E: Adds VX to I. VF is not affected.
                    V[0xF] = (I + V[getX()] > 0xFFF ? 1 : 0);
                    I += V[getX()];
                    pc += 2;
                }
                    break;
                case 0x029: {// FX29: Sets I to the location of the sprite for the character in VX.
                    //Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    I = V[getX()] * 0x5;
                    pc += 2;
                }
                    break;
                default: {
                    unknownOpcode();
                }
            }
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
    if (delay_timer > 0) {
        --
                delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "BEEP!" <<
                      std::endl;
        }
        --
                sound_timer;
    }
//Timers count down at 60Hz, we need to implement something that slow down the emulation cycle
//Execute 60 opcodes in one second
}

unsigned short Core::getX() {
    //ex. opcode = 0x6801;
    //0110100000000001
    //0000111100000000
    //----------------
    //100000000000 >> 8 (shift right 8 bits, which removes 8 zeros)
    //1000 (8 in decimal/hex)
    return (opcode & 0x0F00) >> 8;
}

unsigned short Core::getY() {
    //ex. opcode = 0x5560;
    //0101010101100000
    //0000000011110000
    //----------------
    //0000000001100000 >> 4 (shift right 4 bits, which removes 4 zeros)
    //1111 (15 in decimal/ F in hex)
    return (opcode & 0x00F0) >> 4;
}

unsigned short Core::getNN() {
    return (opcode & 0x00FF);
}

unsigned short Core::getNNN() {
    return (opcode & 0x0FFF);
}

unsigned short Core::getKey() {
    //TODO: Implement getKey
    return 0xF;
}

void Core::unknownOpcode() {
    std::cerr << "Unknown opcode: " << opcode << std::endl;
    exit(1);
}