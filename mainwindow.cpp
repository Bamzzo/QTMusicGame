// mainwindow.cpp
#include "mainwindow.h"
#include "gamescene.h"

#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("MUsicGame");
    resize(900, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setSpacing(30);

    layout->addStretch();

    // 原始按钮
    startButton = new QPushButton("开始游戏", this);
    startButton->setFixedSize(200, 60);
    layout->addWidget(startButton, 0, Qt::AlignHCenter);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::handleStartClicked);

    exitButton = new QPushButton("退出", this);
    exitButton->setFixedSize(200, 60);
    layout->addWidget(exitButton, 0, Qt::AlignHCenter);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::handleExitClicked);

    // 新增四个难度按钮（初始隐藏）
    easyButton = new QPushButton("EASY", this);
    normalButton = new QPushButton("NORMAL", this);
    hardButton = new QPushButton("HARD", this);
    extremeButton = new QPushButton("EXTREME", this);

    QList<QPushButton*> buttons = {easyButton, normalButton, hardButton, extremeButton};
    for (auto btn : buttons) {
        btn->setFixedSize(200, 50);
        btn->hide();
        layout->addWidget(btn, 0, Qt::AlignHCenter);
    }

    connect(easyButton, &QPushButton::clicked, this, &MainWindow::startEasy);
    connect(normalButton, &QPushButton::clicked, this, &MainWindow::startNormal);
    connect(hardButton, &QPushButton::clicked, this, &MainWindow::startHard);
    connect(extremeButton, &QPushButton::clicked, this, &MainWindow::startExtreme);

    layout->addStretch();
}

MainWindow::~MainWindow() {}

void MainWindow::handleStartClicked()
{
    // 隐藏原按钮，显示难度选择
    startButton->hide();
    exitButton->hide();

    easyButton->show();
    normalButton->show();
    hardButton->show();
    extremeButton->show();
}

void MainWindow::handleExitClicked()
{
    close();
}

void MainWindow::showMainMenu()
{
    if (gameScene) {
        gameScene->hide();
        gameScene->deleteLater();
        gameScene = nullptr;
    }

    // 难度选择重置
    easyButton->hide();
    normalButton->hide();
    hardButton->hide();
    extremeButton->hide();

    startButton->show();
    exitButton->show();

    this->show();
}

// 👇 各难度启动
void MainWindow::startEasy()
{
    this->hide();
    gameScene = new GameScene();
    gameScene->loadChart("easy");
    gameScene->resize(this->size());
    gameScene->show();
    connect(gameScene, &GameScene::backToMenu, this, &MainWindow::showMainMenu);
}

void MainWindow::startNormal()
{
    this->hide();
    gameScene = new GameScene();
    gameScene->loadChart("normal");
    gameScene->resize(this->size());
    gameScene->show();
    connect(gameScene, &GameScene::backToMenu, this, &MainWindow::showMainMenu);
}

void MainWindow::startHard()
{
    this->hide();
    gameScene = new GameScene();
    gameScene->loadChart("hard");
    gameScene->resize(this->size());
    gameScene->show();
    connect(gameScene, &GameScene::backToMenu, this, &MainWindow::showMainMenu);
}

void MainWindow::startExtreme()
{
    this->hide();
    gameScene = new GameScene();
    gameScene->loadChart("extreme");
    gameScene->resize(this->size());
    gameScene->show();
    connect(gameScene, &GameScene::backToMenu, this, &MainWindow::showMainMenu);
}
