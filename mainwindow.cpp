#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    this->setWindowFlags(MainWindowFlags);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setWindowOpacity(1);
    this->setStyleSheet(("\
        background-color: rgb" + background.toString() + ";\
        color: white;\
    ").c_str());
    this->resize(400, 24 + 10 + 20 + 40 + 20);

    // center on primary monitor
    {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        this->move((width - this->width()) / 2, (height - this->height()) / 2);
    }

    // minimize, drag to move, exit, etc.
    this->addTopbar();
    this->addContent();

    // animations
    this->animationTimer = new QTimer();
    this->animationTimer->setInterval(5);
    connect(this->animationTimer, &QTimer::timeout, this, /* TODO: use SLOT instead of this garbage: */ [=]() {this->animationUpdate(); });
    this->animationTimer->start();

    // init work
    this->worker = new Worker();
    connect(this->worker, &Worker::progress, this, &MainWindow::workerProgress);
    connect(this->worker, &Worker::description, this, &MainWindow::workerDescription);
    connect(this->worker, &Worker::finished, this, &MainWindow::close);
    this->worker->start();
};

MainWindow::~MainWindow()
{
    delete this->closeButtonOverlay;
    delete this->closeButton;
    delete this->minimizeButtonOverlay;
    delete this->minimizeButton;
    delete this->topbar;
}

void MainWindow::addTopbar()
{
    // topbar
    this->topbar = new QWidget(this);
    this->topbar->setStyleSheet(("background-color: rgb" + this->topbarBackground.toString() + "; border: none;").c_str());
    this->topbar->resize(this->width(), 24);
    this->topbar->installEventFilter(this);

    // close button
    this->closeButtonAnim = Animation::newAnimation("close-hovered", 0);
    this->closeButton = new QPushButton(this->topbar);
    this->closeButtonOverlay = new Overlay("X", this->closeButton);
    this->closeButton->resize(30, 24);
    this->closeButton->setStyleSheet("background-color: rgba(0,0,0,1); border: none;");
    this->closeButton->show();
    this->closeButton->move(this->width() - this->closeButton->width(), 0);
    this->closeButton->installEventFilter(this);

    // minimize button
    this->minimizeButtonAnim = Animation::newAnimation("minimize-hovered", 0);
    this->minimizeButton = new QPushButton(this->topbar);
    this->minimizeButtonOverlay = new Overlay("-", this->minimizeButton);
    this->minimizeButton->resize(30, 24);
    this->minimizeButton->setStyleSheet("background-color: rgba(0,0,0,1); border: none;");
    this->minimizeButton->show();
    this->minimizeButton->move(this->width() - this->closeButton->width() - this->minimizeButton->width(), 0);
    this->minimizeButton->installEventFilter(this);

    // Label
    this->windowLabel = new QLabel("A4G4 UPDATOR", this->topbar);
    this->windowLabel->move(7, 0);
    this->windowLabel->resize(this->width() - 67, this->topbar->height());
    this->windowLabel->setStyleSheet("color: rgb(120, 120, 120); font-size: 10pt; font-family: 'Arial'; font-weight: 700; font-style: italic");
};

void MainWindow::addContent()
{
    int y = this->topbar->height();

    // title
    y += 10;
    this->titleLabel = new QLabel("Updating A4G4... 0%", this);
    this->titleLabel->setStyleSheet("font-size: 12pt; font-family: 'Arial'; font-weight: 500; color: white;");
    this->titleLabel->move(10, y);
    this->titleLabel->resize(this->width() - 20, 20);
    this->titleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    y += 20;

    // draw progress bar
    y += 40;

    // description
    this->descriptionLabel = new QLabel("Initializing...", this);
    this->descriptionLabel->setStyleSheet("font-size: 10pt; font-family: 'Arial'; font-weight: 300; color: rgb(150,150,150);");
    this->descriptionLabel->move(10, y);
    this->descriptionLabel->resize(this->width() - 20, 15);
    this->descriptionLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    y += 5;
}

bool MainWindow::eventFilter(QObject* object, QEvent* e)
{
    if ((QPushButton*)object == this->closeButton)
    {
        switch (e->type())
        {
        case QEvent::Enter:
            this->closeButtonAnim->Set(1);
            return true;
        case QEvent::Leave:
            this->closeButtonAnim->Set(0);
            return true;
        case QEvent::MouseButtonPress:
            this->close();
            return true;
        default: // shut up warnings in qt creator
            break;
        }
    }
    else if ((QPushButton*)object == this->minimizeButton)
    {
        switch (e->type())
        {
        case QEvent::Enter:
            this->minimizeButtonAnim->Set(1);
            return true;
        case QEvent::Leave:
            this->minimizeButtonAnim->Set(0);
            return true;
        case QEvent::MouseButtonPress:
            this->setWindowState(Qt::WindowMinimized);
            return true;
        default: // shut up warnings in qt creator
            break;
        }
    }
    else if ((QWidget*)object == this->topbar)
    {
        QPoint mpos = QCursor::pos();
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        {
            this->isDraggingWindow = true;
            this->draggingOffet = (mpos - this->pos());
            return true;
        }
        case QEvent::MouseButtonRelease:
        {
            this->isDraggingWindow = false;
            return true;
        }
        case QEvent::MouseMove:
        {
            if (!this->isDraggingWindow)
                break;

            this->move(mpos - this->draggingOffet);
            return true;
        }
        default: // shut up warnings in qt creator
            break;
        }
    }

    // if we don't have a special case, do normal thing
    return QWidget::eventFilter(object, e);
}

void MainWindow::animationUpdate()
{
    if (this->closeButtonAnim)
    {
        double timeSinceChange = Animation::age(this->closeButtonAnim);
        float factor = Animation::animate(timeSinceChange, 0.1, Animation::Interpolation::linear);
        if (this->closeButtonAnim->state == 0) factor = 1.f - factor;

        std::string css = "\
            background-color: rgb" + this->topbarBackground.lerp(this->closeButtonHovered, factor).toString() + ";\
            border: none;\
        ";

        this->closeButton->setStyleSheet(css.c_str());
    }
    if (this->minimizeButtonAnim)
    {
        double timeSinceChange = Animation::age(this->minimizeButtonAnim);
        float factor = Animation::animate(timeSinceChange, 0.1, Animation::Interpolation::linear);
        if (this->minimizeButtonAnim->state == 0) factor = 1.f - factor;

        std::string css = "\
            background-color: rgb" + this->topbarBackground.lerp(this->minimizeButtonHovered, factor).toString() + ";\
            border: none;\
        ";

        this->minimizeButton->setStyleSheet(css.c_str());
    }
}

void MainWindow::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen line(Qt::white, 1.f, Qt::SolidLine);
    painter.setPen(line);

    int basey = this->topbar->height() + 33;
    painter.drawRect(QRect(10, basey, this->width() - 20, 31));

    painter.fillRect(QRect(10, basey, (int)(this->progress * (this->width() - 20)), 31), QBrush(Qt::white));
}

Overlay::Overlay(const char* type, QWidget* parent) : QWidget(parent) {
    this->move(0, 0);
    this->_overlay_type = type;
};
void Overlay::paintEvent(QPaintEvent* e)
{
    QWidget* parent = (QWidget*)this->parent();
    this->resize(parent->width(), parent->height());

    int size = 8; // pixels
    int x = (parent->width() - size) / 2;
    int y = (parent->height() - size) / 2;


    QPainter painter(this);


    switch (this->_overlay_type[0])
    {
    case 'x':
    case 'X':
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPen line(Qt::white, 1.25f, Qt::SolidLine);
        painter.setPen(line);

        painter.drawLine(x, y, x + size, y + size);
        painter.drawLine(x + size, y, x, y + size);
        break;
    }
    case '-':
    case '_':
    {
        painter.setRenderHint(QPainter::Antialiasing, false);
        QPen line(Qt::white, 1, Qt::SolidLine);
        painter.setPen(line);

        painter.drawLine(x, y + size / 2, x + size, y + size / 2);
        break;
    }
    }

}

void MainWindow::workerProgress(float p)
{
    if (p >= 0)
    {
        this->titleLabel->setText(("Updating A4G4... " + std::to_string((int)(p * 100.f + 0.5f)) + "%").c_str());
        this->progress = p;
    }
    else
    {
        this->titleLabel->setText("Update failed. Please re-install.");
        this->progress = 1.f;
    }
    this->repaint();
}

void MainWindow::workerDescription(QString value)
{
    this->descriptionLabel->setText(value);
}