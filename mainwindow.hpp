#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QLabel>
#include <QDebug>

#include <string>
#include <algorithm>
#include <Windows.h>

#include "animation.hpp"
#include "color.hpp"
#include "worker.hpp"

constexpr Qt::WindowFlags MainWindowFlags = Qt::WindowType::FramelessWindowHint;

class Overlay : public QWidget
{
    Q_OBJECT
private:
    const char* _overlay_type = "x";

public:
    Overlay(const char* type, QWidget* parent);

    void paintEvent(QPaintEvent* e);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QTimer* animationTimer;

    QWidget* topbar;

    QPushButton* closeButton;
    Overlay* closeButtonOverlay;
    Animation::Anim* closeButtonAnim;

    QPushButton* minimizeButton;
    Overlay* minimizeButtonOverlay;
    Animation::Anim* minimizeButtonAnim;

    QLabel* titleLabel;
    QLabel* descriptionLabel;
    
    QLabel* windowLabel;

    Worker* worker;
    float progress = 0.f;

    bool isDraggingWindow = false;
    QPoint draggingOffet;

    Color background = Color(60, 60, 60);
    Color topbarBackground = Color(20, 20, 20);
    Color closeButtonHovered = Color(200, 50, 50);
    Color minimizeButtonHovered = Color(100, 100, 100);

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject* object, QEvent* e);

    void addTopbar();
    void addContent();

    void paintEvent(QPaintEvent* e);

public slots:
    void animationUpdate();
    void workerProgress(float p);
    void workerDescription(QString value);
};
