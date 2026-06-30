// System Headers
#include "chrono"
#include "thread"

// Local Headers
#include "Core.h"
#include "Renderer.h"
#include "Audio.h"
#include "Input.h"

Core core;
Renderer renderer;

int main(int argc, char *args[]) {
    //The rom to run is passed as the first argument
    if (argc < 2) {
        std::cerr << "Usage: " << args[0] << " <rom>" << std::endl;
        return 1;
    }

    //Initialize Audio
    SDL_Init(SDL_INIT_AUDIO);
    Audio::open();

    //Initialize the core, load rom into memory
    core.Initialize();
    core.LoadGame(args[1]);

    renderer.Initialize(core.GetWidth(), core.GetHeight());

    // The CPU runs at a fixed clock speed while the display and timers run at
    // 60Hz. Both are driven by the real elapsed time so the emulation speed
    // stays consistent regardless of how long a frame takes to process.
    const double cpuClockHz = 700.0;
    const auto frameDuration = std::chrono::duration<double>(1.0 / 60.0);

    bool gameActive = true;
    auto previousTime = std::chrono::steady_clock::now();
    double cycleBudget = 0.0;

    while (gameActive) {
        auto frameStart = std::chrono::steady_clock::now();

        // Work out how many CPU cycles to run based on the time since the last
        // frame. Clamp the delta so a long pause doesn't trigger a huge catch-up.
        std::chrono::duration<double> elapsed = frameStart - previousTime;
        previousTime = frameStart;
        double deltaSeconds = elapsed.count() > 0.25 ? 0.25 : elapsed.count();
        cycleBudget += deltaSeconds * cpuClockHz;

        while (cycleBudget >= 1.0) {
            core.EmulateCycle();
            cycleBudget -= 1.0;
        }

        //Timers and sound tick at 60Hz, once per frame
        core.UpdateTimers();

        renderer.Update(core.Graphics, core.DrawFlag);
        if (core.DrawFlag) {
            core.DrawFlag = false;
        }

        int status = Input::UpdateInput(core.Key);
        if (status != 0) {
            gameActive = false;
        }

        Audio::SetPlaying(core.BeepFlag);

        // Sleep for whatever is left of the frame to hold ~60 frames per second
        std::chrono::duration<double> frameTime = std::chrono::steady_clock::now() - frameStart;
        if (frameTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - frameTime);
        }
    }
    renderer.Terminate();

    return 0;
}