#include "AudioManager.h"

#include "utils/Paths.h"

#include <QEventLoop>
#include <QDir>
#include <QTimer>
#include <QUrl>

AudioManager& AudioManager::instance()
{
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager()
{
    loadSound(m_splashScreen, "splashscreen.wav");
    loadSound(m_switchMenuItem, "switchmenuitem.wav");

    auto waitForLoaded = [](QSoundEffect &sound)
    {
        if (sound.status() == QSoundEffect::Ready)
        {
            return;
        }

        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);

        QObject::connect(&sound, &QSoundEffect::statusChanged, &loop, [&loop, &sound]() {
            if (sound.status() == QSoundEffect::Ready || sound.status() == QSoundEffect::Error)
            {
                loop.quit();
            }
        });
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

        timeout.start(1000);
        loop.exec();
    };

    waitForLoaded(m_splashScreen);
    waitForLoaded(m_switchMenuItem);

    m_musicPlayer.setAudioOutput(&m_musicOutput);
    m_musicOutput.setVolume(0.3f);

    // loadSound(m_move, "move.wav");
    // loadSound(m_select, "select.wav");
    // loadSound(m_back, "back.wav");
    // loadSound(m_startup, "startup.wav");
}

void AudioManager::loadSound(QSoundEffect& sound, const QString& filename)
{
    const QString path = QDir(Paths::audio()).filePath(filename);

    sound.setSource(QUrl::fromLocalFile(path));
    sound.setVolume(m_volume);
}

void AudioManager::play(QSoundEffect& sound)
{
    if (sound.source().isEmpty())
        return;

    sound.stop();
    sound.play();
}

void AudioManager::setVolume(float volume)
{
    m_volume = volume;

    m_splashScreen.setVolume(volume);
    m_switchMenuItem.setVolume(volume);

    m_move.setVolume(volume);
    m_select.setVolume(volume);
    m_back.setVolume(volume);
    m_startup.setVolume(volume);
}

void AudioManager::playSplashScreen()
{
    play(m_splashScreen);
}

void AudioManager::playSwitchMenuItem()
{
    play(m_switchMenuItem);
}

void AudioManager::playMove()
{
    play(m_move);
}

void AudioManager::playSelect()
{
    play(m_select);
}

void AudioManager::playBack()
{
    play(m_back);
}

void AudioManager::playStartup()
{
    play(m_startup);
}


//================== MUSIC ==================//

void AudioManager::playHomeMusic()
{
    if (m_musicPlayer.playbackState() == QMediaPlayer::PlayingState) {
        return;
    }

    const QString path = QDir(Paths::audio()).filePath(QStringLiteral("soundtrackmenu.wav"));

    if (m_musicPlayer.source().toLocalFile() != path) {
        m_musicPlayer.setSource(QUrl::fromLocalFile(path));
    }

    m_musicPlayer.setLoops(QMediaPlayer::Infinite);

    m_musicPlayer.play();
}

void AudioManager::stopHomeMusic()
{
    m_musicPlayer.stop();
}