#include "LibretroAudio.h"
#include "utils/FrameTimingProfiler.h"

#include <QAudioFormat>
#include <QMediaDevices>
#include <algorithm>

QAudioSink* LibretroAudio::m_audioSink = nullptr;
QIODevice* LibretroAudio::m_outputDevice = nullptr;
float LibretroAudio::m_volume = 0.75f;

void LibretroAudio::initialize(unsigned sampleRate)
{
    shutdown();

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    m_audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format);
    m_audioSink->setVolume(m_volume);

    m_audioSink->setBufferSize(8192);

    m_outputDevice = m_audioSink->start();
}

void LibretroAudio::shutdown()
{
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_outputDevice = nullptr;
}

size_t LibretroAudio::pushSamples(const int16_t* data, size_t frames)
{
    FrameTimingProfiler::ScopedTimer timer(FrameTimingProfiler::Stage::Audio);

    if (!m_outputDevice)
        return 0;
    
    const qint64 bytes = static_cast<qint64>(frames * 2 * sizeof(int16_t));
    
    qint64 free = m_audioSink->bytesFree();
    
    if (free < bytes) {
        return frames;
    }

    m_outputDevice->write(
        reinterpret_cast<const char*>(data),
        bytes);

    return frames;
}

void LibretroAudio::setVolume(float volume)
{
    m_volume = std::clamp(volume, 0.0f, 1.0f);

    if (m_audioSink) {
        m_audioSink->setVolume(m_volume);
    }
}

float LibretroAudio::volume()
{
    return m_volume;
}