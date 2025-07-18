#include "effectmanager.h"
#include <QDateTime>

EffectManager::EffectManager()
{
}

void EffectManager::triggerHitEffect(int lane, const QPointF &pos, int currentCombo)
{
    // === 1. 粒子爆炸 ===
    const int maxParticles = 500;
    if (particles.size() < maxParticles) {
        QColor color;
        switch (lane) {
        case 0: color = QColor(0,255,255); break;
        case 1: color = QColor(255,255,0); break;
        case 2: color = QColor(255,255,0); break;
        case 3: color = QColor(0,128,255); break;
        }
        for (int i = 0; i < 15; ++i) {
            particles.append(Particle(pos, color));
        }
    }

    // === 2. 激光扫描（combo 达到条件时触发）===
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (currentCombo >= 30 && now - lastLaserTime > 300) { // 冷却 300ms
        lastLaserTime = now;

        QColor laneColors[4] = {
            QColor(0,255,255),
            QColor(255,255,0),
            QColor(255,255,0),
            QColor(0,128,255)
        };
        int colorIndex = QRandomGenerator::global()->bounded(4);
        QColor chosenColor = laneColors[colorIndex];

        Laser l;
        l.color = chosenColor;
        l.progress = 0.0;

        // 长度至少是屏幕对角线的 1.5 倍，保证覆盖
        qreal diag = 2000.0; // 默认值，具体宽高由 GameScene 决定时可以调整
        l.length = diag * 1.5;
        qreal offset = 200;

        int dir = QRandomGenerator::global()->bounded(8); // 0~7 方向
        int w = 1920;  // 默认宽
        int h = 1080;  // 默认高

        if (dir == 0) {
            l.angle = 0.0;
            l.startCenter = QPointF(w/2, -offset);
            l.endCenter   = QPointF(w/2, h+offset);
        } else if (dir == 1) {
            l.angle = 0.0;
            l.startCenter = QPointF(w/2, h+offset);
            l.endCenter   = QPointF(w/2, -offset);
        } else if (dir == 2) {
            l.angle = M_PI/2;
            l.startCenter = QPointF(-offset, h/2);
            l.endCenter   = QPointF(w+offset, h/2);
        } else if (dir == 3) {
            l.angle = M_PI/2;
            l.startCenter = QPointF(w+offset, h/2);
            l.endCenter   = QPointF(-offset, h/2);
        } else if (dir == 4) {
            l.angle = 45.0 * M_PI / 180.0;
            l.startCenter = QPointF(w + offset, -offset);
            l.endCenter   = QPointF(-offset, h + offset);
        } else if (dir == 5) {
            l.angle = 45.0 * M_PI / 180.0;
            l.startCenter = QPointF(-offset, h + offset);
            l.endCenter   = QPointF(w + offset, -offset);
        } else if (dir == 6) {
            l.angle = -45.0 * M_PI / 180.0;
            l.startCenter = QPointF(w + offset, h + offset);
            l.endCenter   = QPointF(-offset, -offset);
        } else if (dir == 7) {
            l.angle = -45.0 * M_PI / 180.0;
            l.startCenter = QPointF(-offset, -offset);
            l.endCenter   = QPointF(w + offset, h + offset);
        }

        lasers.append(l);
    }
}

void EffectManager::update()
{
    // === 更新粒子 ===
    for (int i = particles.size() - 1; i >= 0; --i) {
        particles[i].update();
        if (particles[i].isDead()) {
            particles.removeAt(i);
        }
    }

    // === 更新激光 ===
    for (int i = lasers.size() - 1; i >= 0; --i) {
        lasers[i].progress += 0.01;
        if (lasers[i].progress > 1.0) {
            lasers.removeAt(i);
        }
    }
}

void EffectManager::draw(QPainter &painter)
{
    // === 绘制粒子 ===
    for (const Particle &particle : particles) {
        particle.draw(&painter);
    }

    // === 绘制激光 ===
    for (const Laser &l : lasers) {
        qreal cx = (1.0 - l.progress) * l.startCenter.x() + l.progress * l.endCenter.x();
        qreal cy = (1.0 - l.progress) * l.startCenter.y() + l.progress * l.endCenter.y();
        QPointF dir(std::cos(l.angle), std::sin(l.angle));
        QPointF p1 = QPointF(cx, cy) - dir * (l.length / 2.0);
        QPointF p2 = QPointF(cx, cy) + dir * (l.length / 2.0);
        QColor c = l.color;
        c.setAlpha(60);
        QPen pen(c, 4, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(p1, p2);
    }
}
