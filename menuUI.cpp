#include "menuUI.h"

MenuUI::MenuUI(QWidget *parent)
    : QWidget(parent)
{
    // 竖直布局，后续可以自己加背景图片
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("选择难度", this);
    title->setAlignment(Qt::AlignCenter);
    QFont font = title->font();
    font.setPointSize(24);
    title->setFont(font);

    easyButton = new QPushButton("Easy", this);
    normalButton = new QPushButton("Normal", this);
    hardButton = new QPushButton("Hard", this);
    extremeButton = new QPushButton("Extreme", this);
    exitButton = new QPushButton("退出", this);

    layout->addWidget(title);
    layout->addWidget(easyButton);
    layout->addWidget(normalButton);
    layout->addWidget(hardButton);
    layout->addWidget(extremeButton);
    layout->addWidget(exitButton);
    layout->setAlignment(Qt::AlignCenter);

    connect(easyButton,    &QPushButton::clicked, this, &MenuUI::onStartEasy);
    connect(normalButton,  &QPushButton::clicked, this, &MenuUI::onStartNormal);
    connect(hardButton,    &QPushButton::clicked, this, &MenuUI::onStartHard);
    connect(extremeButton, &QPushButton::clicked, this, &MenuUI::onStartExtreme);
    connect(exitButton,    &QPushButton::clicked, this, &MenuUI::onExit);
}

void MenuUI::onStartEasy()     { emit startGameRequested("easy"); }
void MenuUI::onStartNormal()   { emit startGameRequested("normal"); }
void MenuUI::onStartHard()     { emit startGameRequested("hard"); }
void MenuUI::onStartExtreme()  { emit startGameRequested("extreme"); }
void MenuUI::onExit()          { emit exitRequested(); }
