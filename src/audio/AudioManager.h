#pragma once

#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>

class AudioManager
{
public:
    static AudioManager& instance();

    void playSplashScreen();
    void playSwitchMenuItem();

    void playMove();
    void playSelect();
    void playBack();
    void playStartup();

    void setVolume(float volume);

    void playHomeMusic();
    void stopHomeMusic();

private:
    AudioManager();

    void loadSound(QSoundEffect& sound, const QString& filename);
    void play(QSoundEffect& sound);

private:
    float m_volume = 0.6f;

    QMediaPlayer m_musicPlayer;
    QAudioOutput m_musicOutput;
    
    QSoundEffect m_splashScreen;
    QSoundEffect m_switchMenuItem;

    QSoundEffect m_move;
    QSoundEffect m_select;
    QSoundEffect m_back;
    QSoundEffect m_startup;
};