#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// WAV file header generator
void writeWav(const std::string& filename, const std::vector<int16_t>& samples, int sampleRate = 44100) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to create " << filename << std::endl;
        return;
    }

    uint32_t dataSize = samples.size() * sizeof(int16_t);
    uint32_t fileSize = 36 + dataSize;

    // RIFF header
    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&fileSize), 4);
    out.write("WAVE", 4);

    // fmt subchunk
    out.write("fmt ", 4);
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels = 1;
    uint32_t byteRate = sampleRate * numChannels * sizeof(int16_t);
    uint16_t blockAlign = numChannels * sizeof(int16_t);
    uint16_t bps = 16;

    out.write(reinterpret_cast<const char*>(&fmtSize), 4);
    out.write(reinterpret_cast<const char*>(&audioFormat), 2);
    out.write(reinterpret_cast<const char*>(&numChannels), 2);
    out.write(reinterpret_cast<const char*>(&sampleRate), 4);
    out.write(reinterpret_cast<const char*>(&byteRate), 4);
    out.write(reinterpret_cast<const char*>(&blockAlign), 2);
    out.write(reinterpret_cast<const char*>(&bps), 2);

    // data subchunk
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&dataSize), 4);

    // actual sample data
    out.write(reinterpret_cast<const char*>(samples.data()), dataSize);
    out.close();
}

int main() {
    // 1. Click Sound (high pitch pop)
    std::vector<int16_t> click_samples;
    for (int i = 0; i < 44100 / 20; ++i) { // 50ms
        float t = (float)i / 44100.0f;
        float s = std::sin(2.0f * M_PI * 1000.0f * t) * std::exp(-t * 80.0f);
        click_samples.push_back((int16_t)(s * 32767.0f));
    }
    writeWav("assets/click.wav", click_samples);

    // 2. Siren Sound (European style hi-low)
    std::vector<int16_t> siren_samples;
    for (int i = 0; i < 44100; ++i) { // 1 second loop
        float t = (float)i / 44100.0f;
        float freq = (i < 44100 / 2) ? 650.0f : 800.0f;
        float s = std::sin(2.0f * M_PI * freq * t) * 0.3f;
        siren_samples.push_back((int16_t)(s * 32767.0f));
    }
    writeWav("assets/siren.wav", siren_samples);

    // 3. Honk Sound (discordant frequencies)
    std::vector<int16_t> honk_samples;
    for (int i = 0; i < 44100 / 2; ++i) { // 0.5s
        float t = (float)i / 44100.0f;
        float env = (t < 0.4f) ? 1.0f : std::exp(-(t - 0.4f) * 10.0f);
        float wave1 = std::sin(2.0f * M_PI * 400.0f * t);
        float wave2 = std::sin(2.0f * M_PI * 440.0f * t);
        float wave3 = std::sin(2.0f * M_PI * 450.0f * t);
        float s = (wave1 + wave2 + wave3) / 3.0f * env * 0.5f;
        honk_samples.push_back((int16_t)(s * 32767.0f));
    }
    writeWav("assets/honk.wav", honk_samples);

    std::cout << "Generated C++ WAV files in assets/\n";
    return 0;
}
