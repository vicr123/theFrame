#include "tutorialwindow.h"
#include "ui_tutorialwindow.h"

#include <QPainter>
#include <QPointer>
#include <tvariantanimation.h>
#include <tsettings.h>

struct TutorialWindowPrivate {
    Qt::Edge pointerEdge;

    QPointer<QWidget> highlightedWidget;
    QPointer<QWidget> highlightedWidgetWindow;
    TutorialWindow::TutorialType type = TutorialWindow::Idle;

    tSettings settings;
};

TutorialWindow::TutorialWindow() :
    QWidget(nullptr),
    ui(new Ui::TutorialWindow)
{
    ui->setupUi(this);

    d = new TutorialWindowPrivate();
    this->setAttribute(Qt::WA_TranslucentBackground);

    connect(qApp, &QApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            if (d->type != TutorialWindow::Idle) {
                this->show(d->highlightedWidget, d->pointerEdge == Qt::RightEdge || d->pointerEdge == Qt::LeftEdge ? Horizontal : Vertical);
            }
        } else {
            this->QWidget::hide();
        }
    });
}

TutorialWindow::~TutorialWindow()
{
    delete ui;
}

void TutorialWindow::trigger(TutorialWindow::TutorialType type, PopoverSide side, QWidget* widget)
{
    QString title;
    QString body;
    switch (type) {
        case TutorialWindow::Idle:
            break;
        case TutorialWindow::AddElement:
            title = tr("Let's get started!");
            body = tr("Go ahead and add an element to get started.");
            break;
        case TutorialWindow::AddTimelineElement:
            title = tr("Let's animate!");
            body = tr("Drag on the timeline to set an animation for a property.");
            break;
        case TutorialWindow::ChangeProperty:
            title = tr("Great!");
            body = tr("Now that you've created an animation, you can change its properties.\nGo ahead and change the end value now.");
            break;
        case TutorialWindow::ChangeEasing:
            title = tr("Make it natural!");
            body = tr("You can make your animation look smoother and more natural by applying an easing curve.\nGo ahead and set an easing curve now.");
            break;
    }

    instance()->ui->titleLabel->setText(title.toUpper());
    instance()->ui->bodyLabel->setText(body);
    instance()->show(widget, side);
    instance()->d->type = type;
}

void TutorialWindow::hide(TutorialWindow::TutorialType type)
{
    if (type == instance()->d->type) {
        if (instance()->d->highlightedWidget) instance()->d->highlightedWidget->removeEventFilter(instance());
        instance()->d->type = Idle;
        instance()->QWidget::hide();
    }
}

TutorialWindow* TutorialWindow::instance()
{
    static TutorialWindow* instance = new TutorialWindow();;
    return instance;
}

void TutorialWindow::show(QWidget* widget, TutorialWindow::PopoverSide side)
{
    if (d->type != Idle) {
        if (d->highlightedWidgetWindow) d->highlightedWidgetWindow->removeEventFilter(this);
        if (d->highlightedWidget) d->highlightedWidget->removeEventFilter(this);
    }

    if (!widget) return;

    d->highlightedWidget = widget;
    d->highlightedWidgetWindow = widget->window();

    widget->installEventFilter(this);
    widget->window()->installEventFilter(this);

    this->setParent(widget->parentWidget());
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QWidget::show();

    QRect widgetRect;
    widgetRect.setSize(widget->visibleRegion().boundingRect().size());
    widgetRect.moveTopLeft(widget->mapToGlobal(widget->visibleRegion().boundingRect().topLeft()));

    QRect parentRect;
    parentRect.setSize(widget->window()->size());
    parentRect.moveTopLeft(widget->window()->mapToGlobal(QPoint(0, 0)));

    QRect thisRect;
    thisRect.setWidth(this->sizeHint().width());
    thisRect.setHeight(this->sizeHint().height());
    thisRect.moveCenter(widgetRect.center());

    if (side == Horizontal) {
        if (widgetRect.center().x() > parentRect.center().x()) {
            thisRect.moveRight(widgetRect.left() - SC_DPI(9));
            d->pointerEdge = Qt::RightEdge;
        } else {
            thisRect.moveLeft(widgetRect.right() + SC_DPI(9));
            d->pointerEdge = Qt::LeftEdge;
        }
    } else {
        if (widgetRect.center().y() > parentRect.center().y()) {
            thisRect.moveBottom(widgetRect.top() - SC_DPI(9));
            d->pointerEdge = Qt::BottomEdge;
        } else {
            thisRect.moveTop(widgetRect.bottom() + SC_DPI(9));
            d->pointerEdge = Qt::TopEdge;
        }
    }

    this->setGeometry(thisRect);
}

bool TutorialWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (d->type != Idle) {
        this->show(d->highlightedWidget, d->pointerEdge == Qt::RightEdge || d->pointerEdge == Qt::LeftEdge ? Horizontal : Vertical);
    }
    return false;
}

void TutorialWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setBrush(this->palette().color(QPalette::Window));
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);
}

void TutorialWindow::on_okButton_clicked()
{
    this->hide(d->type);
}

void TutorialWindow::on_disableTutorialsButton_clicked()
{
    d->settings.setValue("Tutorials/DisableTutorials", true);
    d->settings.sync();

    ui->okButton->click();
}
