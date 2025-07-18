// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

class GameScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleStartClicked();
    void handleExitClicked();
    void showMainMenu();

    // 🎵 新增难度按钮槽
    void startEasy();
    void startNormal();
    void startHard();
    void startExtreme();

private:
    QPushButton *startButton;
    QPushButton *exitButton;

    QPushButton *easyButton;
    QPushButton *normalButton;
    QPushButton *hardButton;
    QPushButton *extremeButton;

    GameScene *gameScene = nullptr;
};

#endif // MAINWINDOW_H
