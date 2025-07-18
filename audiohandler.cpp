#include "audiohandler.h"
#include <QUrl>
#include <QDebug>

AudioHandler::AudioHandler(QObject *parent)
    : QObject(parent)
{
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    audioOutput->setVolume(0.8);  // 默认音量
}

void AudioHandler::playMusicFromStart()
{
    QString musicPath = "qrc:/assets/assets/music/kami.wav";
    player->setSource(QUrl(musicPath));

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia) {
            player->play();
            qDebug() << " 播放开始：" << musicPath;
        }
        if (status == QMediaPlayer::InvalidMedia) {
            qDebug() << "音频加载失败：" << player->errorString();
        }
    });

    qDebug() << "🎵 正在加载音频：" << musicPath;
}


void AudioHandler::pauseMusic()
{
    if (player->playbackState() == QMediaPlayer::PlayingState)
        player->pause();
}

void AudioHandler::resumeMusic()
{
    if (player->playbackState() == QMediaPlayer::PausedState)
        player->play();
}

void AudioHandler::stopMusic()
{
    player->stop();
}

qint64 AudioHandler::getCurrentTime() const
{
    return player->position();
}
