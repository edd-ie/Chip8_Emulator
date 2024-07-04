//
// Created by _edd.ie_ on 23/06/2024.
//

#include "Chip8.h"

#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include "Chip8.h"

class Chip8
{
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

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

        if (const uint8_t byte = opcode & 0x00FFu; registers[Vx] != byte)
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

    /**
    * Jump to register 0 + nnn
    */
    void OP_Bnnn()
    {
        const uint16_t address = opcode & 0x0FFFu;
        pc = registers[0] + address;

    }

    /**
    * set Vx = random byte AND kk
    */
    void OP_Cxkk()
    {

        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t byte = opcode & 0x00FFu;
        registers[Vx] = randByte(randGen) & byte;
    }

    /**
    * Iterate over the sprite, row by row and column by column.
    * 8 columns because sprite is 8px wide.
    * If a sprite pixel is ON then there may be a collision with what’s already being displayed
    * Check if our screen pixel in the same location is set.
    * If so we must set the VF register to express collision.
    * XOR the screen pixel with 0xFFFFFFFF to essentially XOR it with the sprite pixel
    */
    void OP_Dxyn()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        const uint8_t height = opcode & 0x000Fu;

        // Wrap if going beyond screen boundaries
        const uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
        const uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

        registers[0xF] = 0;

        for (unsigned int row = 0; row < height; ++row)
        {
            const uint8_t spriteByte = memory[index + row];

            for (unsigned int col = 0; col < 8; ++col)
            {
                const uint8_t spritePixel = spriteByte & (0x80u >> col);
                uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

                // Sprite pixel is on
                if (spritePixel)
                {
                    // Screen pixel also on - collision
                    if (*screenPixel == 0xFFFFFFFF)
                    {
                        registers[0xF] = 1;
                    }

                    // Effectively XOR with the sprite pixel
                    *screenPixel ^= 0xFFFFFFFF;
                }
            }
        }
    }

    /**
     * Skip next instruction if key with the value of Vx is pressed.
     */
    void OP_Ex9E()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        if (const uint8_t key = registers[Vx]; keypad[key])
        {
            pc += 2;
        }
    }

    /**
     * Skip next instruction if key with the value of Vx is not pressed.
     */
    void OP_ExA1()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        if (const uint8_t key = registers[Vx]; !keypad[key])
        {
            pc += 2;
        }
    }

    /**
    * Set Vx = delay timer value.
    */
    void OP_Fx07()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        registers[Vx] = delayTimer;
    }

    /**
    * Wait for a key press, store the value of the key in Vx.
    * The easiest way to “wait” is to decrement the PC by 2
    * whenever a keypad value is not detected.
    * This has the effect of running the same instruction repeatedly.
     */
    void OP_Fx0A()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if (keypad[0])
        {
            registers[Vx] = 0;
        }
        else if (keypad[1])
        {
            registers[Vx] = 1;
        }
        else if (keypad[2])
        {
            registers[Vx] = 2;
        }
        else if (keypad[3])
        {
            registers[Vx] = 3;
        }
        else if (keypad[4])
        {
            registers[Vx] = 4;
        }
        else if (keypad[5])
        {
            registers[Vx] = 5;
        }
        else if (keypad[6])
        {
            registers[Vx] = 6;
        }
        else if (keypad[7])
        {
            registers[Vx] = 7;
        }
        else if (keypad[8])
        {
            registers[Vx] = 8;
        }
        else if (keypad[9])
        {
            registers[Vx] = 9;
        }
        else if (keypad[10])
        {
            registers[Vx] = 10;
        }
        else if (keypad[11])
        {
            registers[Vx] = 11;
        }
        else if (keypad[12])
        {
            registers[Vx] = 12;
        }
        else if (keypad[13])
        {
            registers[Vx] = 13;
        }
        else if (keypad[14])
        {
            registers[Vx] = 14;
        }
        else if (keypad[15])
        {
            registers[Vx] = 15;
        }
        else
        {
            pc -= 2;
        }
    }

    /**
     * Set delay timer = Vx
     */
    void OP_Fx15()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        delayTimer = registers[Vx];
    }

    /**
     * Set sound timer = Vx
     */
    void OP_Fx18()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        soundTimer = registers[Vx];
    }

    /**
     * Increment index by Vx
     */
    void OP_Fx1E()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        index += registers[Vx];
    }

    /**
    * Set I = location of sprite for digit Vx.
    * We know the font characters are located at 0x50,
    * and we know they’re five bytes each,
    * so we can get the address of the first byte of any character
    * by taking an offset from the start address.
     */
    void OP_Fx29()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        const uint8_t digit = registers[Vx];

        index = FONTSET_START_ADDRESS + (5 * digit);
    }

    /**
    * The interpreter takes the decimal value of Vx,
    * and places the hundreds digit in memory at location in I,
    * the tens digit at location I+1, and the ones digit at location I+2.
    * We can use the modulus operator to get the right-most digit of a number,
    * and then do a division to remove that digit.
    * A division by ten will either completely remove the digit (340 / 10 = 34),
    * or result in a float which will be truncated (345 / 10 = 34.5 = 34).
     */
    void OP_Fx33()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t value = registers[Vx];

        // Ones-place
        memory[index + 2] = value % 10;
        value /= 10;

        // Tens-place
        memory[index + 1] = value % 10;
        value /= 10;

        // Hundreds-place
        memory[index] = value % 10;
    }

    /**
     * Store registers V0 through Vx in memory starting at location I.
     */
    void OP_Fx55()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i)
        {
            memory[index + i] = registers[i];
        }
    }

    /**
     * Read registers V0 through Vx from memory starting at location I.
     */
    void OP_Fx65()
    {
        const uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i)
        {
            registers[i] = memory[index + i];
        }
    }

};

