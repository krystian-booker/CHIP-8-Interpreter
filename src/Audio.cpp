//https://github.com/qxxxb/sdl2-beeper
#include <iostream>
#include <string>
#include <math.h>

#include "Audio.h"

SDL_AudioDeviceID Audio::m_audioDevice;
SDL_AudioSpec Audio::m_obtainedSpec;
double Audio::m_frequency = 392.00;
int Audio::m_pos;

void (*Audio::m_writeData)(uint8_t *ptr, double data);

int (*Audio::m_calculateOffset)(int sample, int channel);

// Calculate the offset in bytes from the start of the audio stream to the
// memory address at `sample` and `channel`.
// Channels are interleaved.
int calculateOffset_s16(int sample, int channel) {
    return (sample * sizeof(int16_t) * Audio::m_obtainedSpec.channels) + (channel * sizeof(int16_t));
}

// Convert a normalized data value (range: 0.0 .. 1.0) to a data value matching
// the audio format.
void writeData_s16(uint8_t *ptr, double data) {
    auto *ptrTyped = (int16_t *) ptr;
    double range = (double) INT16_MAX - (double) INT16_MIN;
    double dataScaled = data * range / 2.0;
    *ptrTyped = dataScaled;
}

// Generate a single sample of the sine wave tone.
double Audio::getData() {
    auto sampleRate = (double) (m_obtainedSpec.freq);

    // Units: samples
    double period = sampleRate / m_frequency;

    // Reset m_pos when it reaches the start of a period so it doesn't run off
    // to infinity (though this won't happen unless you are playing sound for a
    // very long time)
    if (m_pos % (int) period == 0) {
        m_pos = 0;
    }

    double pos = m_pos;
    double angular_freq = (1.0 / period) * 2.0 * M_PI;

    return sin(pos * angular_freq);
}

void Audio::audioCallback(void *userdata, uint8_t *stream, int len) {
    // Unused parameters
    (void) userdata;
    (void) len;

    // Write data to the entire buffer by iterating through all samples and
    // channels.
    for (int sample = 0; sample < m_obtainedSpec.samples; ++sample) {
        double data = getData();
        m_pos++;

        // Write the same data to all channels
        for (int channel = 0; channel < m_obtainedSpec.channels; ++channel) {
            int offset = m_calculateOffset(sample, channel);
            uint8_t *ptrData = stream + offset;
            m_writeData(ptrData, data);
        }
    }
}

void Audio::open() {
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);

    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S16;
    desiredSpec.samples = 512;
    desiredSpec.channels = 1;
    desiredSpec.callback = Audio::audioCallback;

    m_audioDevice = SDL_OpenAudioDevice(
            nullptr, // default device
            0, // playback, not capture
            &desiredSpec,
            &m_obtainedSpec,
            0 // don't allow format changes
    );

    if (m_audioDevice == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    }
    m_writeData = writeData_s16;
    m_calculateOffset = calculateOffset_s16;
}

// Start or stop the tone without blocking the main loop. SDL keeps filling the
// audio buffer via audioCallback while the device is unpaused.
void Audio::SetPlaying(bool playing) {
    SDL_PauseAudioDevice(m_audioDevice, playing ? 0 : 1);
}

void Audio::close() {
    SDL_CloseAudioDevice(m_audioDevice);
}