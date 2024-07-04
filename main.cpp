#include <iostream>
#include <chrono>
#include "Chip8.cpp"
#include "Platform.cpp"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    const int videoScale = std::stoi(argv[1]);
    const int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];

    Platform platform("CHIP-8 Emulator",
        static_cast<int>(VIDEO_WIDTH) * videoScale,
        static_cast<int>(VIDEO_HEIGHT) * videoScale,
        VIDEO_WIDTH,
        VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadROM(romFilename);

    constexpr int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit)
    {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > static_cast<float>(cycleDelay))
        {
            lastCycleTime = currentTime;

            chip8.Cycle();

            platform.Update(chip8.video, videoPitch);
        }
    }

    return 0;
}
