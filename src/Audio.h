//https://github.com/qxxxb/sdl2-beeper
#include <SDL2/SDL.h>
#include "chrono"
#include "thread"

// This class is a singleton, which is bad practice. However, this makes the
// implementation more straightforward.
class Audio {
public:
    static void open(); // Open the audio device
    static void close(); // Close the audio device

    static void Beep();
    static SDL_AudioSpec m_obtainedSpec;

private:
    static SDL_AudioDeviceID m_audioDevice;
    static double m_frequency; // Units: Hz

    // The current playback position, according to `getData()` and
    // `audioCallback()`. Units: samples
    static int m_pos;

    // Pointer to function for offset calculate. Differs between different
    // audio formats.
    static int (*m_calculateOffset)(int sample, int channel);

    // Pointer to function for writing data. Differs between different audio
    // formats.
    static void (*m_writeData)(uint8_t *ptr, double data);

    // Called by `audioCallback` to generate audio data.
    static double getData();

    // This is function is called repeatedly by SDL2 to send data to the audio
    // device.
    static void audioCallback(void *userdata, uint8_t *stream, int len);
};