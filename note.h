#ifndef NOTE_H
#define NOTE_H

enum class NoteType {
    Tap,
    HoldStart,
    HoldEnd
};

class Note
{
public:
    Note(int t, int l, NoteType ty)
        : time(t), lane(l), type(ty), hit(false), holdEndTime(-1), screenY(0) {}

    int  getTime()       const { return time; }
    int  getLane()       const { return lane; }
    NoteType getType()   const { return type; }

    bool isHit()         const { return hit; }
    void setHit(bool v)        { hit = v; }

    // Hold-note utilities
    void setHoldEndTime(int t) { holdEndTime = t; }
    int  getHoldEndTime() const { return holdEndTime; }

    // ✅ 新增：绘制前缓存的 Y 坐标
    void setScreenY(int y) { screenY = y; }
    int getScreenY() const { return screenY; }

private:
    int time;            // ms since chart start
    int lane;            // 0-3
    NoteType type;
    bool hit;

    // valid only for HoldStart
    int holdEndTime;

    // ✅ 新增：用于倒计时阶段的固定绘制位置
    int screenY;
};

#endif // NOTE_H
