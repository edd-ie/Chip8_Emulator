//
// Created by _edd.ie_ on 23/06/2024.
//

#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>

class Chip8
{
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;
    const unsigned int START_ADDRESS = 0x200;
    const unsigned int FONTSET_SIZE = 80;
    const unsigned int FONTSET_START_ADDRESS = 0x50;

    uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

public:
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keypad[16]{};
    uint32_t video[64 * 32]{};
    uint16_t opcode{};


    Chip8():randGen(std::chrono::system_clock::now().time_since_epoch().count())
    {
        //Initialize the program counter
        pc=START_ADDRESS;

        // Initialize RNG
        randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

        // Load fonts into memory
        for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
        {
            memory[FONTSET_START_ADDRESS + i] = fontset[i];
        }
    }

    /**
     * Loading ROM content
     * @param filename rom
     */
    void LoadROM(char const* filename)
    {
        // Open the file as a stream of binary and move the file pointer to the end
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (file.is_open())
        {
            // Get size of file and allocate a buffer to hold the contents
            std::streampos size = file.tellg();
            char* buffer = new char[size];

            // Go back to the beginning of the file and fill the buffer
            file.seekg(0, std::ios::beg);
            file.read(buffer, size);
            file.close();

            // Load the ROM contents into the Chip8's memory, starting at 0x200
            for (long i = 0; i < size; ++i)
            {
                memory[START_ADDRESS + i] = buffer[i];
            }

            // Free the buffer
            delete[] buffer;
        }
    }

    //Instruction set

    /**
     * Clear the display.
     */
    void OP_00E0()
    {
        memset(video, 0, sizeof(video));
    }

    /**
     * Return from subroutine
     */
    void OP_00EE()
    {
        pc = stack[--sp];
    }

    /**
     * Jump to address nnn
     */
    void OP_1nnn()
    {
        const uint16_t address = opcode & 0x0FFFu;
        pc = address;
    }

    /**
     * Call subroutine at nnn
     */
    void OP_2nnn()
    {
        const uint16_t address = opcode & 0x0FFFu;

        stack[sp++] = pc;
        pc = address;
    }

    /**
     * Skip to next instruction if Vx = kk
     */
    void OP_3xkk()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] == byte)
        {
            pc += 2;
        }
    }

    /**
     * Skip to next instruction if Vx != kk
     */
    void OP_4xkk()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] != byte)
        {
            pc += 2;
        }
    }

    /**
     * Skip to next instruction if Vx == Vy
     */
    void OP_5xy0()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] == registers[Vy])
        {
            pc += 2;
        }
    }

    /**
     * Set Vx = kk
     */
    void OP_6xkk()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t byte = opcode & 0x00FFu;

        registers[Vx] = byte;
    }

    /**
     * Add kk to Vx
     */
    void OP_7xkk()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t byte = opcode & 0x00FFu;

        registers[Vx] += byte;
    }

    /**
     * Set Vx = Vy
     */
    void OP_8xy0(){
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] = registers[Vy];
    }

    /**
     * Set Vx = Vx OR Vy
     */
    void OP_8xy1()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] |= registers[Vy];
    }

    /**
     * Set Vx = Vx AND Vy
     */
    void OP_8xy2()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] &= registers[Vy];
    }

    /**
     * Set Vx = Vx XOR Vy
     */
    void OP_8xy3()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] ^= registers[Vy];
    }

    /**
     * The values of Vx and Vy are added together.
     * If the result is greater than 8 bits (i.e., > 255,) VF is set to 1,
     * otherwise 0.
     * Only the lowest 8 bits of the result are kept, and stored in Vx.
     */
    void OP_8xy4()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        const uint16_t sum = registers[Vx] + registers[Vy];

        if (sum > 255U)
        {
            registers[0xF] = 1;
        }
        else
        {
            registers[0xF] = 0;
        }

        registers[Vx] = sum & 0xFFu;
    }

    /**
    * Set Vx = Vx - Vy
    * If Vx > Vy, then VF is set to 1, otherwise 0.
     */
    void OP_8xy5()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] > registers[Vy])
        {
            registers[0xF] = 1;
        }
        else
        {
            registers[0xF] = 0;
        }

        registers[Vx] -= registers[Vy];
    }

    /**
    * If the least-significant bit of Vx is 1, then VF is set to 1,
    * otherwise 0.
    * Then Vx is divided by 2.
    * A right shift is performed on Vx (division by 2)
     */
    void OP_8xy6()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save LSB in VF
        registers[0xF] = (registers[Vx] & 0x1u);

        registers[Vx] >>= 1;
    }

    /**
     * If Vy > Vx, then VF is set to 1, otherwise 0.
     * Vx is subtracted from Vy, and the results stored in Vx.
     */
    void OP_8xy7()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vy] > registers[Vx])
        {
            registers[0xF] = 1;
        }
        else
        {
            registers[0xF] = 0;
        }

        registers[Vx] = registers[Vy] - registers[Vx];
    }

    /**
    * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
    * Then Vx is multiplied by 2.
    * A left shift is performed (multiplication by 2), and the most significant bit is saved in Register VF.
     */
    void OP_8xyE()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save MSB in VF
        registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

        registers[Vx] <<= 1;
    }

    /**
     * Skip to next instruction if Vx != Vy
     */
    void OP_9xy0()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] != registers[Vy])
        {
            pc += 2;
        }
    }

    /**
     * Set index = address
     */
    void OP_Annn()
    {
        const uint16_t address = opcode & 0x0FFFu;
        index = address;
    }
};



#endif //CHIP8_H
