#include "Core.h"

// Clear the memory, registers and screen
void Core::Initialize() {
    pc = 0x200; //Program counter starts at 0x200
    opcode = 0; //Reset current opcode
    I = 0;      //Reset index register
    sp = 0;     //Reset stack pointer

    //Clear display
    clearDisplay();
    DrawFlag = true;

    //Clear stack
    stack[16] = {0};

    //Clear registers V0-VF
    V[16] = {0};

    //Clear memory
    memory[4096] = {0};

    //Load font set
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    //Reset timers
    delayTimer = 0;
    soundTimer = 0;

    //Random seed
    srand(time(NULL));
}

// Copy the program into the memory
void Core::LoadGame(const char *romName) {
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
        for (int i = 0; i < 4096; i++) {
            std::cout << (int) memory[i] << " ";
        }
    }

    inFile.close();
}

// Emulate one cycle of the system
void Core::EmulateCycle() {
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
                    clearDisplay();
                    DrawFlag = true;
                    pc += 2;
                }
                    break;
                case 0x00E: {// 0x00EE: returns from subroutine
                    if (sp <= 0) {
                        std::cerr << "stack underflow" << std::endl;
                        exit(1);
                    }
                    pc = stack[sp];
                    sp--;
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
            sp++;
            if (sp >= 16) {
                std::cerr << "Stack overflow" << std::endl;
                exit(1);
            }

            stack[sp] = pc + 2;
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

            //TODO: Is this necessary?
            unsigned short v = V[getX()] + getNN();
            if (v > 255) {
                v -= 256;
            }
            V[getX()] = v;
            pc += 2;
        }
            break;
        case 0x8000: {
            switch (opcode & 0x000F) {
                case 0x000: {// 8XY0: Sets VX to the value of VY
                    V[getX()] = V[getY()];
                    pc += 2;
                }
                    break;
                case 0x001: {// 8XY1: Sets VX to VX or VY
                    V[getX()] = (V[getX()] | V[getY()]);
                    pc += 2;
                }
                    break;
                case 0x002: {// 8XY2: Sets VX to VX and VY
                    V[getX()] = (V[getX()] & V[getY()]);
                    pc += 2;
                }
                    break;
                case 0x003: {// 8XY3: Sets VX to VX xor VY
                    V[getX()] = (V[getX()] ^ V[getY()]);
                    pc += 2;
                }
                    break;
                case 0x004: {// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                    //TODO: Add documentation
                    V[0xF] = ((int) V[getX()] + (int) V[getY()]) > 255 ? 1 : 0;
                    V[getX()] += V[getY()];
                    pc += 2;
                }
                    break;
                case 0x005: {// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    //TODO: Add documentation
                    V[0xF] = (V[getX()] > V[getY()]) ? 1 : 0;
                    V[getX()] -= V[getY()];
                    pc += 2;
                }
                    break;
                case 0x006: {// 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                    V[0xF] = V[getX()] & 0x1;
                    V[getX()] = V[getX()] >> 1;
                    pc += 2;
                }
                    break;
                case 0x007: {// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    //TODO: Add documentation
                    V[0xF] = (V[getY()] > V[getX()]) ? 1 : 0;
                    V[getX()] = V[getY()] - V[getX()];
                    pc += 2;
                }
                    break;
                case 0x00E: {// 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                    V[0xF] = (V[getX()] >> 7) & 0x1;
                    V[getX()] = (V[getX()] << 1);
                    pc += 2;
                }
                    break;
                default: {
                    unknownOpcode();
                }
            }
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
        case 0xB000: {// BNNN: Jumps to the address NNN plus V0.
            pc = getNNN() + V[0];
        }
            break;
        case 0xC000: {// CXNN: Sets VX to the result of a bitwise and operation on a random number
            // (Typically: 0 to 255) and NN.
            V[getX()] = ((rand() % 0xFF) & getNN());
            pc += 2;
        }
            break;
        case 0xD000: {// DXYN: Draws a sprite at coordinate (VX, VY)
            //Graphics drawing from
            //http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter#
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(Graphics[(x + xline + ((y + yline) * WIDTH))] == 1)
                            V[0xF] = 1;
                        Graphics[x + xline + ((y + yline) * WIDTH)] ^= 1;
                    }
                }
            }

            DrawFlag = true;
            pc += 2;
        }
            break;
        case 0xE000: {
            switch (opcode & 0x00FF) {
                case 0x09E: {// EX9E: Skips the next instruction if the Key stored in VX is pressed
                    if (V[getX()] == getKey()) {
                        pc += 2;
                    }
                    pc += 2;
                }
                    break;
                case 0x0A1: {// EXA1: Skips the next instruction if the Key stored in VX isn't pressed
                    if (V[getX()] != getKey()) {
                        pc += 2;
                    }
                    pc += 2;
                }
                    break;
                default: {
                    unknownOpcode();
                }
            }
        }
            break;
        case 0xF000: {
            // 1111000000101001 0xF029
            // 0000000011111111 0x00FF
            // ----------------
            // 0000000000101001 0x029
            switch (opcode & 0x00FF) {
                case 0x007: {// FX07: Sets VX to the value of the delay timer
                    V[getX()] = delayTimer;
                    pc += 2;
                }
                    break;
                case 0x00A: {// FX0A: A Key press is awaited, and then stored in VX.
                    // (Blocking Operation. All instruction halted until next Key event)
                    V[getX()] = getKey();

                    pc += 2;
                }
                    break;
                case 0x015: {// FX15: Sets the delay timer to VX
                    delayTimer = V[getX()];
                    pc += 2;
                }
                    break;
                case 0x018: {// FX18: Sets the sound timer to VX
                    soundTimer = V[getX()];
                    pc += 2;
                }
                    break;
                case 0x01E: {// FX1E: Adds VX to I. VF is not affected.
                    I = I + V[getX()];

                    if (I > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }

                    pc += 2;
                }
                    break;
                case 0x029: {// FX29: Sets I to the location of the sprite for the character in VX.
                    //Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    I = V[getX()] * 0x5;
                    pc += 2;
                }
                    break;
                case 0x033: {
                    /*
                     * FX33:
                     * Stores the binary-coded decimal representation of VX, with the most significant
                     * of three digits at the address in I, the middle digit at I plus 1, and the least
                     * significant digit at I plus 2. (In other words, take the decimal representation of VX,
                     * place the hundreds digit in memory at location in I, the tens digit at location I+1,
                     * and the ones digit at location I+2.)
                     */

                    memory[I] = V[getX()] / 100;     // Hundredth's digit
                    memory[I + 1] = (V[getX()] % 100) / 10; // Ten's digit
                    memory[I + 2] = V[getX()] % 10; // One's digit
                    pc += 2;
                }
                    break;
                case 0x055: {// FX55: Stores V0 to VX (including VX) in memory starting at address I
                    //The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                    for (int i = 0; i <= getX(); i++) {
                        memory[I + i] = V[i];
                    }

                    //From original interpreter. Not in modern implementations
                    //I += getX() + 1;
                    pc += 2;
                }
                    break;
                case 0x065: {// FX65: Fills V0 to VX (including VX) with values from memory starting at address I
                    // The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                    for (int i = 0; i <= getX(); i++) {
                        V[i] = memory[I + i];
                    }

                    //From original interpreter. Not in modern implementations
                    //I += getX() + 1;
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
    if (delayTimer > 0) {
        --delayTimer;
    }

    if (soundTimer > 0) {
        if (soundTimer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        --soundTimer;
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

void Core::clearDisplay() {
    for(int i = 0; i < 2048; i++){
        Graphics[i] = 0;
    }
}

void Core::unknownOpcode() {
    std::cerr << "Unknown opcode: " << opcode << std::endl;
    exit(1);
}

int Core::GetWidth(){
    return WIDTH;
}

int Core::GetHeight(){
    return HEIGHT;
}