#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QMap>
#include <array>
#include <QSoundEffect>
#include <QPointF>

// 模块化管理
#include "note.h"
#include "notemanager.h"
#include "particle.h"
#include "audiohandler.h"
#include "effectmanager.h"

class GameScene : public QWidget
{
    Q_OBJECT
public:
    explicit GameScene(QWidget *parent = nullptr);
    void loadChart(const QString &difficulty);

signals:
    void backToMenu();

protected:
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *) override;

private slots:
    void onBackButtonClicked();
    void onPauseButtonClicked();

private:
    // === 模块 ===
    NoteManager noteManager;
    EffectManager effectManager;
    AudioHandler *audioHandler = nullptr;

    // === 状态 ===
    bool gameStarted = false;
    bool isPaused = false;
    bool countdownActive = false;
    bool showResult = false;
    bool pendingEnd = false;
    bool lanePressed[4] = {false, false, false, false};
    bool shouldShake = false;

    // === 分数统计 ===
    int score = 0;
    int combo = 0;
    int maxCombo = 0;
    int perfectCount = 0;
    int greatCount = 0;
    int goodCount = 0;
    int missCount = 0;

    // === 判定文字 ===（直接使用 NoteManager 中定义的结构）
    QVector<JudgementText> judgements;

    // === 音符相关 ===
    std::vector<Note> notes; // 仅用于绘制（未来可移除）
    std::array<qint64, 4> laneFlashTime{};   // 闪光时间戳
    QMap<int, qint64> hitTimestamps;         // 头像显示用时间戳

    // === UI 控件 ===
    QPushButton *backButton = nullptr;
    QPushButton *pauseButton = nullptr;
    QPushButton *retryButton = nullptr;
    QPushButton *menuButton = nullptr;

    QPixmap hitEffects[4];
    QPixmap bgCache;

    // === 时间控制 ===
    QTimer *updateTimer = nullptr;
    qint64 pauseStartTime = 0;
    qint64 totalPausedDuration = 0;
    qint64 countdownStartTime = 0;
    qint64 gameStartTime = 0;
    qint64 endTriggerTime = 0;

    // === 倒计时文字 ===
    QString countdownText;
    QString currentDifficulty;

    // === 轨道灯光动画 ===
    QMap<int, QList<qint64>> laneFlashQueue;

    // === 音效 ===
    QSoundEffect *hitSound = nullptr;

    // === 私有函数 ===
    void triggerHitEffect(int lane);
    void updateParticles();            // 已迁移至 EffectManager，可移除
    void drawParticles(QPainter &p);  // 已迁移至 EffectManager，可移除
    void startCountdown();
};

#endif // GAMESCENE_H
