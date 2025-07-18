#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

// ✅ 粒子类（可以直接用你现有的 particle.h，也可以内嵌）
#include "particle.h"

class EffectManager
{
public:
    EffectManager();

    // ✅ 每次击打时调用，触发特效
    void triggerHitEffect(int lane, const QPointF &pos, int currentCombo);

    // ✅ 每帧逻辑更新（在定时器里调用）
    void update();

    // ✅ 每帧绘制（在 paintEvent 里调用）
    void draw(QPainter &painter);

private:
    // === 激光结构体 ===
    struct Laser {
        QColor color;
        qreal angle;        // 固定角度（弧度）
        qreal length;       // 激光长度
        qreal progress;     // 0.0 ~ 1.0
        QPointF startCenter;
        QPointF endCenter;
    };

    QVector<Particle> particles;
    QVector<Laser> lasers;

    qint64 lastLaserTime = 0; // 用于限制激光冷却，防止频繁生成
};

#endif // EFFECTMANAGER_H
