#include "Core.h"

// Clear the memory, registers and screen
void Core::Initialize() {
    //Build the opcode dispatch tables
    setupTables();

    pc = 0x200; //Program counter starts at 0x200
    opcode = 0; //Reset current opcode
    I = 0;      //Reset index register
    sp = 0;     //Reset stack pointer

    //Clear display
    clearDisplay();
    DrawFlag = true;

    //Clear stack
    for (unsigned short &s : stack) {
        s = 0;
    }

    //Clear registers V0-VF
    for (unsigned char &reg : V) {
        reg = 0;
    }

    //Clear memory
    for (unsigned char &m : memory) {
        m = 0;
    }

    //Load font set
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    //Reset timers
    delayTimer = 0;
    soundTimer = 0;

    //Random seed
    srand(time(nullptr));
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
        for (unsigned char i : memory) {
            std::cout << (int) i << " ";
        }
    }

    inFile.close();
}

// Wire up the function pointer tables. Every slot defaults to unknownOpcode so
// that any undecoded instruction is reported rather than silently ignored.
void Core::setupTables() {
    for (OpcodeHandler &h : mainTable) h = &Core::unknownOpcode;
    for (OpcodeHandler &h : table0) h = &Core::unknownOpcode;
    for (OpcodeHandler &h : table8) h = &Core::unknownOpcode;
    for (OpcodeHandler &h : tableE) h = &Core::unknownOpcode;
    for (OpcodeHandler &h : tableF) h = &Core::unknownOpcode;

    // Primary table, indexed by the high nibble (opcode & 0xF000) >> 12.
    mainTable[0x0] = &Core::dispatch0;
    mainTable[0x1] = &Core::OP_1nnn;
    mainTable[0x2] = &Core::OP_2nnn;
    mainTable[0x3] = &Core::OP_3xnn;
    mainTable[0x4] = &Core::OP_4xnn;
    mainTable[0x5] = &Core::OP_5xy0;
    mainTable[0x6] = &Core::OP_6xnn;
    mainTable[0x7] = &Core::OP_7xnn;
    mainTable[0x8] = &Core::dispatch8;
    mainTable[0x9] = &Core::OP_9xy0;
    mainTable[0xA] = &Core::OP_Annn;
    mainTable[0xB] = &Core::OP_Bnnn;
    mainTable[0xC] = &Core::OP_Cxnn;
    mainTable[0xD] = &Core::OP_Dxyn;
    mainTable[0xE] = &Core::dispatchE;
    mainTable[0xF] = &Core::dispatchF;

    // 0x0 group, indexed by the low nibble (opcode & 0x000F).
    table0[0x0] = &Core::OP_00E0;
    table0[0xE] = &Core::OP_00EE;

    // 0x8 group, indexed by the low nibble (opcode & 0x000F).
    table8[0x0] = &Core::OP_8xy0;
    table8[0x1] = &Core::OP_8xy1;
    table8[0x2] = &Core::OP_8xy2;
    table8[0x3] = &Core::OP_8xy3;
    table8[0x4] = &Core::OP_8xy4;
    table8[0x5] = &Core::OP_8xy5;
    table8[0x6] = &Core::OP_8xy6;
    table8[0x7] = &Core::OP_8xy7;
    table8[0xE] = &Core::OP_8xyE;

    // 0xE group, indexed by the low nibble (opcode & 0x000F).
    tableE[0xE] = &Core::OP_Ex9E; // EX9E
    tableE[0x1] = &Core::OP_ExA1; // EXA1

    // 0xF group, indexed by the low byte (opcode & 0x00FF).
    tableF[0x07] = &Core::OP_Fx07;
    tableF[0x0A] = &Core::OP_Fx0A;
    tableF[0x15] = &Core::OP_Fx15;
    tableF[0x18] = &Core::OP_Fx18;
    tableF[0x1E] = &Core::OP_Fx1E;
    tableF[0x29] = &Core::OP_Fx29;
    tableF[0x33] = &Core::OP_Fx33;
    tableF[0x55] = &Core::OP_Fx55;
    tableF[0x65] = &Core::OP_Fx65;
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

    // Decode and execute: the high nibble selects a handler from the primary
    // table. Groups that share a nibble route through a secondary table.
    (this->*mainTable[(opcode & 0xF000) >> 12])();
}

// Secondary dispatchers ------------------------------------------------------

void Core::dispatch0() {
    (this->*table0[opcode & 0x000F])();
}

void Core::dispatch8() {
    (this->*table8[opcode & 0x000F])();
}

void Core::dispatchE() {
    (this->*tableE[opcode & 0x000F])();
}

void Core::dispatchF() {
    (this->*tableF[opcode & 0x00FF])();
}

// Opcode handlers ------------------------------------------------------------

// 0x00E0: Clears the screen
void Core::OP_00E0() {
    clearDisplay();
    DrawFlag = true;
    pc += 2;
}

// 0x00EE: Returns from subroutine
void Core::OP_00EE() {
    if (sp <= 0) {
        std::cerr << "stack underflow" << std::endl;
        exit(1);
    }
    pc = stack[sp];
    sp--;
}

// 1NNN: Jumps to address NNN
void Core::OP_1nnn() {
    pc = getNNN();
}

// 2NNN: Calls subroutine at NNN
void Core::OP_2nnn() {
    //Store current program counter location
    sp++;
    if (sp >= 16) {
        std::cerr << "Stack overflow" << std::endl;
        exit(1);
    }

    stack[sp] = pc + 2;
    pc = getNNN();
}

// 3XNN: Skips the next instruction if VX equals NN
void Core::OP_3xnn() {
    if (V[getX()] == getNN()) {
        pc += 2;
    }
    pc += 2;
}

// 4XNN: Skips the next instruction if VX doesn't equal NN
void Core::OP_4xnn() {
    if (V[getX()] != getNN()) {
        pc += 2;
    }
    pc += 2;
}

// 5XY0: Skips the next instruction if VX equals VY
void Core::OP_5xy0() {
    if (V[getX()] == V[getY()]) {
        pc += 2;
    }
    pc += 2;
}

// 6XNN: Sets VX to NN
void Core::OP_6xnn() {
    V[getX()] = getNN();
    pc += 2;
}

// 7XNN: Adds NN to VX. (Carry flag is not changed)
void Core::OP_7xnn() {
    unsigned short v = V[getX()] + getNN();
    if (v > 255) {
        v -= 256;
    }
    V[getX()] = v;
    pc += 2;
}

// 8XY0: Sets VX to the value of VY
void Core::OP_8xy0() {
    V[getX()] = V[getY()];
    pc += 2;
}

// 8XY1: Sets VX to VX or VY
void Core::OP_8xy1() {
    V[getX()] = (V[getX()] | V[getY()]);
    pc += 2;
}

// 8XY2: Sets VX to VX and VY
void Core::OP_8xy2() {
    V[getX()] = (V[getX()] & V[getY()]);
    pc += 2;
}

// 8XY3: Sets VX to VX xor VY
void Core::OP_8xy3() {
    V[getX()] = (V[getX()] ^ V[getY()]);
    pc += 2;
}

// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
void Core::OP_8xy4() {
    V[0xF] = ((int) V[getX()] + (int) V[getY()]) > 255 ? 1 : 0;
    V[getX()] += V[getY()];
    pc += 2;
}

// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
void Core::OP_8xy5() {
    V[0xF] = (V[getX()] >= V[getY()]) ? 1 : 0;
    V[getX()] -= V[getY()];
    pc += 2;
}

// 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
void Core::OP_8xy6() {
    V[0xF] = V[getX()] & 0x1;
    V[getX()] = V[getX()] >> 1;
    pc += 2;
}

// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
void Core::OP_8xy7() {
    V[0xF] = (V[getY()] >= V[getX()]) ? 1 : 0;
    V[getX()] = V[getY()] - V[getX()];
    pc += 2;
}

// 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
void Core::OP_8xyE() {
    V[0xF] = (V[getX()] >> 7) & 0x1;
    V[getX()] = (V[getX()] << 1);
    pc += 2;
}

// 9XY0: Skips the next instruction if VX doesn't equal VY
void Core::OP_9xy0() {
    if (V[getX()] != V[getY()]) {
        pc += 2;
    }
    pc += 2;
}

// ANNN: Sets I to the address NNN
void Core::OP_Annn() {
    I = getNNN();
    pc += 2;
}

// BNNN: Jumps to the address NNN plus V0.
void Core::OP_Bnnn() {
    pc = getNNN() + V[0];
}

// CXNN: Sets VX to the result of a bitwise and operation on a random number
// (Typically: 0 to 255) and NN.
void Core::OP_Cxnn() {
    V[getX()] = ((rand() % 256) & getNN());
    pc += 2;
}

// DXYN: Draws a sprite at coordinate (VX, VY)
void Core::OP_Dxyn() {
    //Starting coordinate wraps; pixels past the edge are clipped (CHIP-8 spec).
    unsigned short x = V[(opcode & 0x0F00) >> 8] % WIDTH;
    unsigned short y = V[(opcode & 0x00F0) >> 4] % HEIGHT;
    unsigned short height = opcode & 0x000F;
    unsigned short pixel;

    V[0xF] = 0;
    for (int yline = 0; yline < height; yline++) {
        int py = y + yline;
        if (py >= HEIGHT) break;            // clip rows below the bottom edge
        pixel = memory[I + yline];
        for (int xline = 0; xline < 8; xline++) {
            int px = x + xline;
            if (px >= WIDTH) break;          // clip columns past the right edge
            if ((pixel & (0x80 >> xline)) != 0) {
                int index = px + (py * WIDTH);
                if (Graphics[index] == 1)
                    V[0xF] = 1;
                Graphics[index] ^= 1;
            }
        }
    }

    DrawFlag = true;
    pc += 2;
}

// EX9E: Skips the next instruction if the Key stored in VX is pressed
void Core::OP_Ex9E() {
    if (Key[V[getX()]] != 0) {
        pc += 2;
    }
    pc += 2;
}

// EXA1: Skips the next instruction if the Key stored in VX isn't pressed
void Core::OP_ExA1() {
    if (Key[V[getX()]] == 0) {
        pc += 2;
    }
    pc += 2;
}

// FX07: Sets VX to the value of the delay timer
void Core::OP_Fx07() {
    V[getX()] = delayTimer;
    pc += 2;
}

// FX0A: A Key press is awaited, and then stored in VX.
// (Blocking Operation. All instruction halted until next Key event)
void Core::OP_Fx0A() {
    bool keyPressed = false;

    for (int i = 0; i < 16; i++) {
        if (Key[i] == 1) {
            keyPressed = true;
            V[getX()] = (uint8_t) i;
        }
    }

    if (keyPressed) {
        pc += 2;
    }
}

// FX15: Sets the delay timer to VX
void Core::OP_Fx15() {
    delayTimer = V[getX()];
    pc += 2;
}

// FX18: Sets the sound timer to VX
void Core::OP_Fx18() {
    soundTimer = V[getX()];
    pc += 2;
}

// FX1E: Adds VX to I. VF is not affected.
void Core::OP_Fx1E() {
    I = I + V[getX()];

    if (I > 0xFFF) {
        V[0xF] = 1;
    } else {
        V[0xF] = 0;
    }

    pc += 2;
}

// FX29: Sets I to the location of the sprite for the character in VX.
// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void Core::OP_Fx29() {
    I = V[getX()] * 0x5;
    pc += 2;
}

/*
 * FX33:
 * Stores the binary-coded decimal representation of VX, with the most significant
 * of three digits at the address in I, the middle digit at I plus 1, and the least
 * significant digit at I plus 2. (In other words, take the decimal representation of VX,
 * place the hundreds digit in memory at location in I, the tens digit at location I+1,
 * and the ones digit at location I+2.)
 */
void Core::OP_Fx33() {
    memory[I] = V[getX()] / 100;     // Hundredth's digit
    memory[I + 1] = (V[getX()] % 100) / 10; // Ten's digit
    memory[I + 2] = V[getX()] % 10; // One's digit
    pc += 2;
}

// FX55: Stores V0 to VX (including VX) in memory starting at address I
// The offset from I is increased by 1 for each value written, but I itself is left unmodified.
void Core::OP_Fx55() {
    for (int i = 0; i <= getX(); i++) {
        memory[I + i] = V[i];
    }

    //From original interpreter. Not in modern implementations
    //I += getX() + 1;
    pc += 2;
}

// FX65: Fills V0 to VX (including VX) with values from memory starting at address I
// The offset from I is increased by 1 for each value written, but I itself is left unmodified.
void Core::OP_Fx65() {
    for (int i = 0; i <= getX(); i++) {
        V[i] = memory[I + i];
    }

    //From original interpreter. Not in modern implementations
    //I += getX() + 1;
    pc += 2;
}

// Decrement the timers. This is decoupled from EmulateCycle so that the CPU
// can run many instructions per frame while the timers tick down at 60Hz.
void Core::UpdateTimers() {
    if (delayTimer > 0) {
        --delayTimer;
    }

    if (soundTimer > 0) {
        --soundTimer;
    }

    // The beeper should sound for the entire duration the sound timer is active.
    BeepFlag = (soundTimer > 0);
}

unsigned short Core::getX() const {
    //ex. opcode = 0x6801;
    //0110100000000001
    //0000111100000000
    //----------------
    //100000000000 >> 8 (shift right 8 bits, which removes 8 zeros)
    //1000 (8 in decimal/hex)
    return (opcode & 0x0F00) >> 8;
}

unsigned short Core::getY() const {
    //ex. opcode = 0x5560;
    //0101010101100000
    //0000000011110000
    //----------------
    //0000000001100000 >> 4 (shift right 4 bits, which removes 4 zeros)
    //1111 (15 in decimal/ F in hex)
    return (opcode & 0x00F0) >> 4;
}

unsigned short Core::getNN() const {
    return (opcode & 0x00FF);
}

unsigned short Core::getNNN() const {
    return (opcode & 0x0FFF);
}

void Core::clearDisplay() {
    for (unsigned char &Graphic : Graphics) {
        Graphic = 0;
    }
}

void Core::unknownOpcode() {
    std::cerr << "Unknown opcode: " << opcode << std::endl;
    exit(1);
}

int Core::GetWidth() {
    return WIDTH;
}

int Core::GetHeight() {
    return HEIGHT;
}