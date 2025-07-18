#include "gamescene.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDateTime>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTimer>
#include <cmath>
#include <QRandomGenerator>  // 如果后续有震动特效用到随机数

GameScene::GameScene(QWidget *parent)
    : QWidget(parent)
{
    // 返回按钮
    backButton = new QPushButton("返回菜单", this);
    backButton->setGeometry(20, 20, 120, 40);
    backButton->setStyleSheet("color: white; font: 16px 'Microsoft YaHei'; background-color: #444; border: none;");
    connect(backButton, &QPushButton::clicked, this, &GameScene::onBackButtonClicked);

    // 暂停按钮
    pauseButton = new QPushButton("暂停", this);
    pauseButton->setGeometry(width() - 140, 20, 120, 40);
    pauseButton->setStyleSheet("color: white; font: 16px 'Microsoft YaHei'; background-color: #444; border: none;");
    connect(pauseButton, &QPushButton::clicked, this, &GameScene::onPauseButtonClicked);

    // 加载角色头像资源
    hitEffects[0].load(":/assets/assets/images/miku.png");
    hitEffects[1].load(":/assets/assets/images/len.png");
    hitEffects[2].load(":/assets/assets/images/rin.png");
    hitEffects[3].load(":/assets/assets/images/kaito.png");

    for (int i = 0; i < 4; ++i) {
        if (hitEffects[i].isNull())
            qDebug() << "头像图加载失败：轨道" << i;
        else
            qDebug() << "头像图加载成功：" << i << hitEffects[i].size();
    }

    // 打击音效
    hitSound = new QSoundEffect(this);
    hitSound->setSource(QUrl("qrc:/assets/assets/musics/hit.wav"));
    hitSound->setVolume(0.9);

    // 初始化音频播放
    audioHandler = new AudioHandler(this);

    // 初始化游戏状态
    gameStarted = false;
    countdownActive = false;
    showResult = false;
    combo = maxCombo = score = 0;
    perfectCount = greatCount = missCount = goodCount = 0;
    totalPausedDuration = 0;

    // ✅ 渲染优化属性
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    // ✅ 定时器刷新逻辑 + 绘制
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        qint64 currentTime = audioHandler->getCurrentTime();

        // ✅ 长按状态更新 & 每500ms加分
        noteManager.updateHoldPulses(currentTime, lanePressed,
                                     score, combo, maxCombo, perfectCount, judgements);

        // ✅ 检查漏判
        noteManager.checkMissNotes(currentTime,
                                   combo, missCount, judgements);

        // ✅ 特效更新
        effectManager.update();

        // 请求重绘
        update();
    });
    updateTimer->start(4);

    // ✅ 启动倒计时
    QTimer::singleShot(100, this, [this]() {
        startCountdown();
    });
}

void GameScene::loadChart(const QString &difficulty)
{
    currentDifficulty = difficulty;

    // ✅ 移除重试和菜单按钮
    if (retryButton) {
        retryButton->hide();
        retryButton->deleteLater();
        retryButton = nullptr;
    }
    if (menuButton) {
        menuButton->hide();
        menuButton->deleteLater();
        menuButton = nullptr;
    }

    // ✅ 重置状态
    laneFlashQueue.clear();
    hitTimestamps.clear();
    score = combo = maxCombo = 0;
    perfectCount = greatCount = goodCount = missCount = 0;
    showResult = false;
    gameStarted = false;
    countdownActive = false;

    // ✅ 调用 NoteManager 加载谱面
    QString path = ":/assets/assets/notes/kami_" + difficulty + "_full.txt";
    if (!noteManager.loadChart(path)) {
        qDebug() << "❌ 加载谱面失败：" << path;
        return;
    }

    // ⭐⭐⭐ 同步 NoteManager 的音符数据到 GameScene 的 notes，用于绘制和计算结算
    notes = noteManager.getNotes();

    // ✅ 重新启动倒计时和音符逻辑
    startCountdown();
}



void GameScene::onBackButtonClicked()
{
    emit backToMenu();
}

void GameScene::onPauseButtonClicked()
{
    if (!isPaused) {
        pauseStartTime = QDateTime::currentMSecsSinceEpoch();
        updateTimer->stop();
        audioHandler->pauseMusic();
        isPaused = true;
        pauseButton->setText("继续");
    } else {
        qint64 resumeTime = QDateTime::currentMSecsSinceEpoch();
        totalPausedDuration += resumeTime - pauseStartTime;
        updateTimer->start(16);
        audioHandler->resumeMusic();
        isPaused = false;
        pauseButton->setText("暂停");
    }
}

void GameScene::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (backButton)
        backButton->setGeometry(20, 20, 120, 40);
    if (pauseButton)
        pauseButton->setGeometry(width() - 140, 20, 120, 40);

    // ✅ 根据当前窗口大小重建轨道背景缓存
    int laneWidth = width() / 4;
    int laneHeight = height();
    bgCache = QPixmap(size());
    bgCache.fill(Qt::black);
    QPainter bgPainter(&bgCache);
    for (int i = 0; i < 4; ++i) {
        QRect laneRect(i * laneWidth, 0, laneWidth, laneHeight);
        bgPainter.fillRect(laneRect, (i % 2 == 0) ? QColor(20,20,20) : QColor(50,50,50));
    }
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat() || isPaused) return;

    int lane = -1;
    if (event->key() == Qt::Key_W) lane = 0;
    else if (event->key() == Qt::Key_E) lane = 1;
    else if (event->key() == Qt::Key_I) lane = 2;
    else if (event->key() == Qt::Key_O) lane = 3;
    if (lane == -1) return;

    // 防止重复判定
    if (lanePressed[lane]) return;
    lanePressed[lane] = true;
    laneFlashTime[lane] = QDateTime::currentMSecsSinceEpoch();

    qint64 currentTime = audioHandler->getCurrentTime();

    // ✅ 调用 NoteManager 判定
    if (noteManager.handleKeyPress(lane, currentTime,
                                   score, combo, maxCombo,
                                   perfectCount, greatCount, goodCount, missCount,
                                   judgements)) {
        triggerHitEffect(lane);
        hitTimestamps[lane] = currentTime;
    }
}





void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat() || isPaused) return;

    int lane = -1;
    if (event->key() == Qt::Key_W) lane = 0;
    else if (event->key() == Qt::Key_E) lane = 1;
    else if (event->key() == Qt::Key_I) lane = 2;
    else if (event->key() == Qt::Key_O) lane = 3;
    if (lane == -1) return;

    lanePressed[lane] = false; // 解除按下状态

    qint64 currentTime = audioHandler->getCurrentTime();

    // ✅ 调用 NoteManager 判定 HoldEnd
    if (noteManager.handleKeyRelease(lane, currentTime,
                                     score, combo, maxCombo,
                                     perfectCount, greatCount, goodCount, missCount,
                                     judgements)) {
        triggerHitEffect(lane);
        hitTimestamps[lane] = currentTime;
    }
}





void GameScene::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    // 背景缓存
    painter.drawPixmap(0, 0, bgCache);

    // 震动特效
    if (shouldShake) {
        int shakeStrength = 2;
        int dx = QRandomGenerator::global()->bounded(-shakeStrength, shakeStrength + 1);
        int dy = QRandomGenerator::global()->bounded(-shakeStrength, shakeStrength + 1);
        painter.translate(dx, dy);
        shouldShake = false;
    }

    int laneWidth = width() / 4;
    int laneHeight = height();
    int judgmentLineY = laneHeight - 100;

    qint64 currentTime = (!gameStarted) ? 0 : audioHandler->getCurrentTime();

    // 判定线
    painter.setBrush(QColor(255,255,255));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, judgmentLineY, width(), 5);

    // 倒计时
    if (countdownActive) {
        painter.setOpacity(1.0);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 60, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, countdownText);
    }

    // 轨道按下持续亮光反馈
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 4; ++i) {
        if (lanePressed[i] || (now - laneFlashTime[i] < 80)) {
            QColor c;
            if (i == 0) c = QColor(0,255,255);
            if (i == 1) c = QColor(255,255,0);
            if (i == 2) c = QColor(255,255,0);
            if (i == 3) c = QColor(0,128,255);
            c.setAlpha(80);
            painter.setBrush(c);
            painter.setPen(Qt::NoPen);
            painter.drawRect(QRect(i * laneWidth, 0, laneWidth, height()));
        }
    }

    // ⭐ 使用 noteManager 的 notes 进行绘制，保证判定后能实时消失
    const auto &drawNotes = noteManager.getNotes();

    // Tap 音符
    for (const Note &note : drawNotes) {
        if (note.getType() != NoteType::Tap || note.isHit()) continue;
        int lane = note.getLane();
        qint64 time = note.getTime();
        int y = judgmentLineY - (time - currentTime) * 0.55;
        if (y < -100 || y > height() + 100) continue;
        int x = lane * laneWidth + laneWidth / 4;
        int w = laneWidth / 2;
        int h = 20;
        QRect noteRect(x, y, w, h);
        QColor baseColor;
        switch (lane) {
        case 0: baseColor = QColor(0,255,255); break;
        case 1: baseColor = QColor(255,255,0); break;
        case 2: baseColor = QColor(255,255,0); break;
        case 3: baseColor = QColor(0,128,255); break;
        }
        // 拖尾效果
        for (int i = 0; i < 3; ++i) {
            QColor trailColor = baseColor;
            trailColor.setAlphaF(0.1 * (3 - i));
            QRect trailRect = noteRect.adjusted(0, -i * 12, 0, -i * 12);
            painter.setBrush(trailColor);
            painter.setPen(Qt::NoPen);
            painter.drawRect(trailRect);
        }
        painter.setBrush(baseColor);
        painter.setPen(Qt::NoPen);
        painter.drawRect(noteRect);
    }

    // Hold 音符
    double timeSec = currentTime / 1000.0;
    double pulse = (std::sin(timeSec * 1.5 * 3.14159) + 1.0) / 2.0;
    double alphaVal = 180 + pulse * 60;
    double scaleVal = 1.0 + 0.03 * pulse;
    // 长按绘制
    for (size_t i = 0; i < drawNotes.size(); ++i) {
        const Note &startNote = drawNotes[i];
        if (startNote.getType() != NoteType::HoldStart) continue;

        for (size_t j = i + 1; j < drawNotes.size(); ++j) {
            const Note &endNote = drawNotes[j];
            if (endNote.getType() != NoteType::HoldEnd || endNote.getLane() != startNote.getLane()) continue;
            if (endNote.getTime() - startNote.getTime() < 200) break;

            int lane = startNote.getLane();
            qint64 startT = startNote.getTime();
            qint64 endT = endNote.getTime();
            if (currentTime > endT + 200) break;

            // 统一速度系数（仍用 0.55）
            double speed = 0.55;
            int y1 = judgmentLineY - static_cast<int>((startT - currentTime) * speed);
            int y2 = judgmentLineY - static_cast<int>((endT   - currentTime) * speed);

            // === 额外防御性限制 ===
            // 先 clamp 在可见范围之外的值，避免过大矩形
            int limitTop = -200;
            int limitBottom = height() + 200;
            y1 = qBound(limitTop, y1, limitBottom);
            y2 = qBound(limitTop, y2, limitBottom);

            int top = qMin(y1, y2);
            int bottom = qMax(y1, y2);
            int baseHeight = bottom - top;

            // 严格过滤异常矩形
            if (baseHeight <= 2) break;
            if (baseHeight > height() + 400) break; // 🚀 加强防御：高度异常时直接跳过
            if (bottom < -60 || top > height()) break;

            int animatedHeight = static_cast<int>(baseHeight * scaleVal);
            int x = lane * laneWidth + laneWidth / 4;
            QRect animatedRect(x,
                               top - (animatedHeight - baseHeight) / 2,
                               laneWidth / 2,
                               animatedHeight);

            QColor holdColor;
            switch (lane) {
            case 0: holdColor = QColor(0,255,255); break;
            case 1: holdColor = QColor(255,255,0); break;
            case 2: holdColor = QColor(255,255,0); break;
            case 3: holdColor = QColor(0,128,255); break;
            }
            holdColor.setAlphaF(alphaVal / 255.0);

            painter.setBrush(holdColor);
            painter.setPen(Qt::NoPen);
            painter.drawRect(animatedRect);

            break; // 找到匹配的 HoldEnd 就退出内层循环
        }
    }

    // 粒子与激光特效
    effectManager.draw(painter);

    // 头像特效
    for (int i = 0; i < 4; ++i) {
        if (hitTimestamps.contains(i)) {
            qint64 delta = currentTime - hitTimestamps[i];
            if (delta < 1000) {
                qreal progress = 1.0 - delta / 1000.0;
                int alpha = static_cast<int>(progress * 255);
                int x = i * laneWidth + laneWidth / 4;
                int y = judgmentLineY - laneWidth / 4;
                QRect effectRect(x, y, laneWidth / 2, laneWidth / 2);
                QPixmap scaled = hitEffects[i].scaled(effectRect.size(),
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation);
                painter.setOpacity(alpha / 255.0);
                painter.drawPixmap(effectRect.topLeft(), scaled);
            }
        }
    }

    // 实时分数
    if (!showResult) {
        painter.setOpacity(1.0);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",16,QFont::Bold));
        painter.drawText(QRect(width()-250,80,200,60), Qt::AlignRight, QString("Score: %1").arg(score));
    }

    // Combo
    if (combo > 0 && !showResult) {
        painter.setOpacity(1.0);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",32,QFont::Bold));
        painter.drawText(rect().adjusted(0,60,0,0), Qt::AlignTop|Qt::AlignHCenter, QString::number(combo) + " Combo");
    }

    // 判定文字
    for (int i = judgements.size()-1; i >= 0; --i) {
        const JudgementText &j = judgements[i];
        qint64 delta = currentTime - j.startTime;
        if (delta > 1000) {
            judgements.remove(i);
            continue;
        }
        qreal progress = delta/1000.0;
        qreal alpha = 1.0 - progress;
        qreal scale = 1.0 + 0.5*progress;
        painter.save();
        painter.setOpacity(alpha);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", static_cast<int>(24*scale), QFont::Bold));
        int x = j.lane * laneWidth + laneWidth/2;
        int y = height() - 160;
        painter.drawText(QRect(x-100,y,200,40), Qt::AlignCenter, j.text);
        painter.restore();
    }

    // 计算最后音符时间
    qint64 lastNoteTime = 0;
    for (const Note &n : drawNotes) {
        if (n.getTime() > lastNoteTime) {
            lastNoteTime = n.getTime();
        }
    }

    if (!showResult && !pendingEnd && currentTime > lastNoteTime + 3000) {
        pendingEnd = true;
        endTriggerTime = QDateTime::currentMSecsSinceEpoch();
    }

    // 渐暗处理
    if (pendingEnd && !showResult) {
        qint64 nowFade = QDateTime::currentMSecsSinceEpoch();
        qint64 delta = nowFade - endTriggerTime;
        qreal progress = qBound(0.0, delta / 3000.0, 1.0);
        painter.setOpacity(progress * 1.0);
        painter.fillRect(rect(), QColor(0,0,0));
        if (delta >= 3000) {
            showResult = true;
            pendingEnd = false;
        }
    }

    // 结算界面
    if (showResult) {
        painter.setOpacity(1.0);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei",42,QFont::Bold));
        painter.drawText(QRect(0,height()/2-160,width(),60), Qt::AlignCenter, "🎵 游戏结束 🎵");

        int totalNotes = drawNotes.size();
        double maxScore = totalNotes * 100.0;
        double percent = (maxScore > 0.0) ? (score / maxScore * 100.0) : 0.0;

        QString rank;
        if (percent >= 90.0)       rank = "S";
        else if (percent >= 80.0)  rank = "A";
        else if (percent >= 70.0)  rank = "B";
        else if (percent >= 60.0)  rank = "C";
        else                       rank = "D";

        painter.setFont(QFont("Microsoft YaHei", 22));
        QStringList lines = {
            QString("Score: %1").arg(score),
            QString("Max Combo: %1").arg(maxCombo),
            QString("Perfect: %1").arg(perfectCount),
            QString("Great: %1").arg(greatCount),
            QString("Good: %1").arg(goodCount),
            QString("Miss: %1").arg(missCount),
            QString("百分比: %1%").arg(percent, 0, 'f', 1)
        };

        int baseY = height() / 2 - 100;
        for (int i = 0; i < lines.size(); ++i) {
            painter.drawText(QRect(0, baseY + i * 30, width(), 30), Qt::AlignCenter, lines[i]);
        }

        painter.setFont(QFont("Microsoft YaHei", 26, QFont::Bold));
        painter.drawText(QRect(0, baseY + lines.size() * 30 + 10, width(), 40),
                         Qt::AlignCenter,
                         QString("评级：%1").arg(rank));

        int buttonY = baseY + lines.size()*30 + 70;
        int buttonWidth = 140;
        int spacing = 20;
        int cx = width() / 2;

        if (!retryButton) {
            retryButton = new QPushButton("再来一局", this);
            retryButton->setFixedSize(buttonWidth, 40);
            retryButton->setStyleSheet("font: 16px 'Microsoft YaHei'; background-color: #eee; border-radius: 6px;");
           connect(retryButton, &QPushButton::clicked, this, &GameScene::backToMenu);
        }
        retryButton->move(cx - buttonWidth - spacing/2, buttonY);
        retryButton->show();

        if (!menuButton) {
            menuButton = new QPushButton("返回菜单", this);
            menuButton->setFixedSize(buttonWidth, 40);
            menuButton->setStyleSheet("font: 16px 'Microsoft YaHei'; background-color: #eee; border-radius: 6px;");
            connect(menuButton, &QPushButton::clicked, this, &GameScene::backToMenu);
        }
        menuButton->move(cx + spacing/2, buttonY);
        menuButton->show();
    }
}




void GameScene::startCountdown()
{
    countdownStartTime = QDateTime::currentMSecsSinceEpoch();
    countdownActive = true;
    gameStarted = false;

    QTimer::singleShot(0,    this, [this]() { countdownText = "3"; update(); });
    QTimer::singleShot(1000, this, [this]() { countdownText = "2"; update(); });
    QTimer::singleShot(2000, this, [this]() { countdownText = "1"; update(); });
    QTimer::singleShot(3000, this, [this]() { countdownText = "Start!"; update(); });

    // Step 1: 隐藏倒计时文字（3.6s）
    QTimer::singleShot(3600, this, [this]() {
        countdownActive = false;
        countdownText.clear();  // 可选，清空文字
        update();
    });

    // Step 2: 播放音乐（3.5s）
    QTimer::singleShot(3300, this, [this]() {
        audioHandler->playMusicFromStart();  // 播放 BGM
    });

    // Step 3: 音符真正开始（4.0s）
    QTimer::singleShot(4000, this, [this]() {
        gameStartTime = QDateTime::currentMSecsSinceEpoch();  // 音符开始动
        gameStarted = true;                                   // ✅ 加上这一句
        update();
    });

}

void GameScene::triggerHitEffect(int lane)
{
    if (hitSound) {
        hitSound->stop();
        hitSound->play();
    }
    int laneWidth = width() / 4;
    int x = lane * laneWidth + laneWidth / 2;
    int y = height() - 100;
    effectManager.triggerHitEffect(lane, QPointF(x, y), combo);
    if (combo >= 20) {
        shouldShake = true;
    }
}




