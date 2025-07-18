#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>

class AudioHandler : public QObject
{
    Q_OBJECT

public:
    explicit AudioHandler(QObject *parent = nullptr);
    void playMusicFromStart();
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    qint64 getCurrentTime() const;

private:
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
};

#endif // AUDIOHANDLER_H
