#ifndef PARTICLE_H
#define PARTICLE_H

#include <QPointF>
#include <QColor>
#include <QPainter>

class Particle
{
public:
    Particle(QPointF position, QColor color);

    void update();                      // 更新粒子
    void draw(QPainter *p) const;       // 绘制粒子（注意加了 const）
    bool isDead() const;                // 判断是否消失

private:
    QPointF pos;
    QPointF velocity;
    QColor color;
    float alpha;
    int life;
};

#endif // PARTICLE_H
