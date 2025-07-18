#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include <vector>
#include <array>
#include <QString>
#include <QVector>
#include <QDateTime>
#include "note.h"

struct HoldState {
    bool   active      = false;
    qint64 lastPulse   = 0;
    qint64 endTime     = 0;
    bool   endJudged   = false;
};

struct JudgementText {
    QString text;
    qint64 startTime;
    int lane;
};

class NoteManager {
public:
    NoteManager();

    // 加载谱面
    bool loadChart(const QString &path);

    // 获取音符列表（只读）
    const std::vector<Note>& getNotes() const { return notes; }

    // 判定相关
    bool handleKeyPress(int lane, qint64 currentTime,
                        int &score, int &combo, int &maxCombo,
                        int &perfectCount, int &greatCount, int &goodCount, int &missCount,
                        QVector<JudgementText> &judgements);

    bool handleKeyRelease(int lane, qint64 currentTime,
                          int &score, int &combo, int &maxCombo,
                          int &perfectCount, int &greatCount, int &goodCount, int &missCount,
                          QVector<JudgementText> &judgements);

    void updateHoldPulses(qint64 currentTime,
                          const bool lanePressed[4],
                          int &score, int &combo, int &maxCombo,
                          int &perfectCount, QVector<JudgementText> &judgements);

    void checkMissNotes(qint64 currentTime,
                        int &combo, int &missCount,
                        QVector<JudgementText> &judgements);

    // Hold 状态
    const std::array<HoldState,4>& getHoldStates() const { return holdStates; }

private:
    std::vector<Note> notes;
    std::array<HoldState,4> holdStates{};
};

#endif // NOTEMANAGER_H
