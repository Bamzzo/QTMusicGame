#ifndef MENUUI_H
#define MENUUI_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class MenuUI : public QWidget
{
    Q_OBJECT
public:
    explicit MenuUI(QWidget *parent = nullptr);

signals:
    void startGameRequested(const QString &difficulty);
    void exitRequested();

private slots:
    void onStartEasy();
    void onStartNormal();
    void onStartHard();
    void onStartExtreme();
    void onExit();

private:
    QPushButton *easyButton;
    QPushButton *normalButton;
    QPushButton *hardButton;
    QPushButton *extremeButton;
    QPushButton *exitButton;
};

#endif // MENUUI_H
