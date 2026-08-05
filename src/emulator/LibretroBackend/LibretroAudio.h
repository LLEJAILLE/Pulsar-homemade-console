#pragma once

#include <QAudioSink>
#include <QIODevice>

class LibretroAudio
{
public:
    static void initialize(unsigned sampleRate);
    static void shutdown();
    static size_t pushSamples(const int16_t* data, size_t frames);

private:
    static QAudioSink* m_audioSink;
    static QIODevice* m_outputDevice;
};