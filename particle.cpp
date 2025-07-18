#include "particle.h"
#include <QRandomGenerator>

Particle::Particle(QPointF position, QColor c)
    : pos(position), color(c), alpha(255), life(40)  // ✅ 延长生命周期
{
    float vx = (QRandomGenerator::global()->bounded(40) - 20) / 10.0;
    float vy = (QRandomGenerator::global()->bounded(40) - 20) / 10.0;
    velocity = QPointF(vx, vy);
}

void Particle::update()
{
    pos += velocity;
    alpha -= 5;   // ✅ 更缓慢淡出，原来是 8
    if (alpha < 0) alpha = 0;
    life--;
}

void Particle::draw(QPainter *p) const
{
    QColor c = color;
    c.setAlphaF(alpha / 255.0);
    p->setRenderHint(QPainter::Antialiasing, true);  // ✅ 仅粒子使用抗锯齿
    p->setBrush(c);
    p->setPen(Qt::NoPen);
    p->drawEllipse(pos, 4, 4);
}

bool Particle::isDead() const
{
    return life <= 0 || alpha <= 0;
}
