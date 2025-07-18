#include "notemanager.h"
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include <QDebug>

NoteManager::NoteManager() {}

bool NoteManager::loadChart(const QString &path)
{
    notes.clear();
    for (int i=0;i<4;++i) holdStates[i] = {};

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ 无法打开谱面文件：" << path;
        return false;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("BPM") || line.startsWith("OFFSET") || line.isEmpty())
            continue;
        QStringList parts = line.split('\t');
        if (parts.size() != 3) continue;
        bool ok1, ok2, ok3;
        int time = parts[0].toInt(&ok1);
        int lane = parts[1].toInt(&ok2);
        int typeCode = parts[2].toInt(&ok3);
        if (!(ok1 && ok2 && ok3)) continue;
        NoteType type;
        if (typeCode==0) type = NoteType::Tap;
        else if (typeCode==1) type = NoteType::HoldStart;
        else if (typeCode==2) type = NoteType::HoldEnd;
        else continue;
        notes.emplace_back(time,lane,type);
    }
    file.close();
    qDebug() << "✅ 成功加载谱面：" << path << "，共加载音符数量：" << notes.size();
    return true;
}

bool NoteManager::handleKeyPress(int lane, qint64 currentTime,
                                 int &score, int &combo, int &maxCombo,
                                 int &perfectCount, int &greatCount, int &goodCount, int &missCount,
                                 QVector<JudgementText> &judgements)
{
    const int hitWindow = 300;
    for (Note &note : notes) {
        if (note.getLane() != lane || note.isHit()) continue;
        qint64 diff = qAbs(note.getTime() - currentTime);
        if (note.getType()==NoteType::Tap && diff<=hitWindow) {
            note.setHit(true);
            QString judge; int addScore=0;
            if(diff<=50){judge="Perfect";perfectCount++;addScore=100;}
            else if(diff<=100){judge="Great";greatCount++;addScore=70;}
            else if(diff<=200){judge="Good";goodCount++;addScore=60;}
            else {judge="Miss";missCount++;combo=0;}
            score+=addScore;
            if(judge!="Miss"){combo++;maxCombo=qMax(combo,maxCombo);}
            judgements.append({judge,currentTime,lane});
            return true;
        }
        if (note.getType()==NoteType::HoldStart && diff<=hitWindow) {
            note.setHit(true);
            QString judge; int addScore=0;
            if(diff<=50){judge="Perfect";perfectCount++;addScore=100;}
            else if(diff<=100){judge="Great";greatCount++;addScore=70;}
            else if(diff<=200){judge="Good";goodCount++;addScore=60;}
            else {judge="Miss";missCount++;combo=0;}
            score+=addScore;
            if(judge!="Miss"){combo++;maxCombo=qMax(combo,maxCombo);}
            judgements.append({judge,currentTime,lane});

            // 找到对应的 HoldEnd
            for (const Note &n: notes) {
                if (n.getLane()==lane && n.getType()==NoteType::HoldEnd && n.getTime()>note.getTime()) {
                    holdStates[lane].active=true;
                    holdStates[lane].lastPulse=currentTime;
                    holdStates[lane].endTime=n.getTime();
                    holdStates[lane].endJudged=false;
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

bool NoteManager::handleKeyRelease(int lane, qint64 currentTime,
                                   int &score, int &combo, int &maxCombo,
                                   int &perfectCount, int &greatCount, int &goodCount, int &missCount,
                                   QVector<JudgementText> &judgements)
{
    const int hitWindow = 300;
    for (Note &note : notes) {
        if (note.getLane()!=lane || note.getType()!=NoteType::HoldEnd || note.isHit()) continue;
        qint64 diff = qAbs(note.getTime()-currentTime);
        if (diff<=hitWindow) {
            note.setHit(true);
            QString judge; int addScore=0;
            if(diff<=50){judge="Perfect";perfectCount++;addScore=100;}
            else if(diff<=100){judge="Great";greatCount++;addScore=70;}
            else if(diff<=200){judge="Good";goodCount++;addScore=60;}
            else {judge="Miss";missCount++;combo=0;}
            score+=addScore;
            if(judge!="Miss"){combo++;maxCombo=qMax(combo,maxCombo);}
            judgements.append({judge,currentTime,lane});
            holdStates[lane].active=false;
            holdStates[lane].endJudged=true;
            return true;
        }
    }
    return false;
}

void NoteManager::updateHoldPulses(qint64 currentTime,
                                   const bool lanePressed[4],
                                   int &score, int &combo, int &maxCombo,
                                   int &perfectCount, QVector<JudgementText> &judgements)
{
    const int tickInterval = 500;
    const int hitWindow = 300;
    for (int lane=0;lane<4;++lane) {
        auto &st = holdStates[lane];
        if (!st.active) continue;
        if (!st.endJudged && st.endTime>0 && lanePressed[lane] && qAbs(currentTime-st.endTime)<=hitWindow) {
            for (Note &n: notes) {
                if (n.getLane()==lane && n.getType()==NoteType::HoldEnd && !n.isHit()) {
                    n.setHit(true); break;
                }
            }
            score+=100; perfectCount++; combo++; maxCombo=qMax(combo,maxCombo);
            judgements.append({"Perfect",currentTime,lane});
            st.endJudged=true; st.active=false;
            continue;
        }
        if (lanePressed[lane] && currentTime<st.endTime-hitWindow) {
            if (currentTime-st.lastPulse>=tickInterval) {
                st.lastPulse=currentTime;
                score+=100; perfectCount++; combo++; maxCombo=qMax(combo,maxCombo);
                judgements.append({"Perfect",currentTime,lane});
            }
        }
    }
}

void NoteManager::checkMissNotes(qint64 currentTime,
                                 int &combo, int &missCount,
                                 QVector<JudgementText> &judgements)
{
    const int hitWindow = 300;
    for (Note &note: notes) {
        if (note.isHit()) continue;
        int lane = note.getLane();
        if (note.getType()==NoteType::Tap) {
            if (currentTime>note.getTime()+hitWindow) {
                note.setHit(true);
                combo=0; missCount++;
                judgements.append({"Miss",currentTime,lane});
            }
            continue;
        }
        if (note.getType()==NoteType::HoldStart) {
            continue;
        }
        if (note.getType()==NoteType::HoldEnd) {
            if (holdStates[lane].active) continue;
            if (currentTime>note.getTime()+hitWindow) {
                note.setHit(true);
                combo=0; missCount++;
                judgements.append({"Miss",currentTime,lane});
                for (Note &start: notes) {
                    if (start.getLane()==lane && start.getType()==NoteType::HoldStart && !start.isHit()) {
                        start.setHit(true); break;
                    }
                }
                holdStates[lane].active=false;
            }
        }
    }
}
