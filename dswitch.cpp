#include "dswitch.h"

#include <QPainter>
#include <QDebug>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QMouseEvent>

#define ANIMATION_DURATION 400
#define BORDER_LINE_WIDTH 1
#define GOLDEN_RATIO 1.6180339875
#define DECREASED_GOLDEN_RATIO 1.38
#define ENABLED_OPACITY 1.0
#define DISABLED_OPACITY 0.5
#define KNOB_BACKGROUND_COLOR QColor(255, 255, 255)
#define DISABLED_BORDER_COLOR QColor(255, 255, 255, 51)
#define DEFAULT_TINT_COLOR QColor(69, 219, 92)
#define INACTIVE_BACKGROUND_COLOR QColor(255, 255, 255, 77)

DLayer::DLayer(QWidget *parent) : QWidget(parent)
{
    // setEnabled(false);
    mCornerRadius = 0;
    mColor = Qt::transparent;
    mOpacity = 1.0;
    mBorderWidth = 1;

    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_PaintUnclipped);
}

DLayer::~DLayer()
{

}

void
DLayer::paintEvent(QPaintEvent *e)
{
    e->accept();

    QPainter g(this);

    g.setRenderHint(QPainter::Antialiasing);
    g.setOpacity(mOpacity);

    if (mColor != Qt::transparent) {
        QPen pen;
        pen.setColor(mColor);
        pen.setWidth(mBorderWidth);
        g.setPen(pen);

        g.setBrush(QBrush(mColor));
    }

    if (mCornerRadius) {
        g.drawRoundedRect(mBorderWidth, mBorderWidth, width() - 2 * mBorderWidth,
            height() - 2 * mBorderWidth, mCornerRadius, mCornerRadius);
    }
    else {
        g.drawRect(mBorderWidth, mBorderWidth, width() - 2 * mBorderWidth,
            height() - 2 * mBorderWidth);
    }
}

DSwitch::DSwitch(QWidget *parent) : QWidget(parent),
    backgroundLayer(NULL),
    knobLayer(NULL)
{
    // resize(40, 25);
    installEventFilter(this);
    setup();
}

DSwitch::~DSwitch()
{
}

void
DSwitch::setup()
{
    enabled = true;
    isOn = false;
    hasDragged = false;
    isDraggingTowardsOn = false;
    isActive = false;
    setupLayers();
}

void
DSwitch::setupLayers()
{
    if (backgroundLayer == NULL)
        backgroundLayer = new DLayer(this);

    backgroundLayer->setFrame(QRect(0, 0, width(), height()));
    backgroundLayer->setBorderWidth(BORDER_LINE_WIDTH);
    backgroundLayer->setColor(DISABLED_BORDER_COLOR);

    if (knobLayer == NULL)
        knobLayer = new DLayer(this);

    knobLayer->setFrame(rectForKnob());
    knobLayer->setColor(KNOB_BACKGROUND_COLOR);

    reloadLayerSize(false);
    reloadLayer(false);
}

void
DSwitch::reloadLayerSize(bool anim, QAnimationGroup *p)
{
    if (anim) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
     
        QPropertyAnimation *anim = new QPropertyAnimation(knobLayer, "frame");
        anim->setEndValue(rectForKnob());
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);

        anim = new QPropertyAnimation(knobLayer, "cornerRadius");
        anim->setEndValue(height() / 2);
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);

        anim = new QPropertyAnimation(backgroundLayer, "cornerRadius");
        anim->setEndValue(height() / 2);
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);

        if (p)
            p->addAnimation(group);
        else
            group->start();
    }
    else {
        knobLayer->setFrame(rectForKnob());
        int radius = height() / 2;
        knobLayer->setCornerRadius(radius);
        backgroundLayer->setCornerRadius(radius);
    }
}

void
DSwitch::reloadLayer(bool anim, QAnimationGroup *parent)
{
    QColor backgroundColor = DISABLED_BORDER_COLOR;
    if ((hasDragged && isDraggingTowardsOn) || (!hasDragged && isOn)) {
        backgroundColor = DEFAULT_TINT_COLOR;
    }

    if (anim) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;

        QPropertyAnimation *anim = new QPropertyAnimation(backgroundLayer, "color");
        anim->setEndValue(backgroundColor);
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);


        anim = new QPropertyAnimation(backgroundLayer, "opacity");
        anim->setEndValue(enabled ? ENABLED_OPACITY : DISABLED_OPACITY);
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);

        anim = new QPropertyAnimation(knobLayer, "frame");
        anim->setEndValue(rectForKnob());
        anim->setDuration(ANIMATION_DURATION);
        group->addAnimation(anim);

        if (parent)
            parent->addAnimation(group);
        else
            group->start();
    }
    else {
        backgroundLayer->setColor(backgroundColor);
        backgroundLayer->setOpacity(enabled ? ENABLED_OPACITY : DISABLED_OPACITY);
        knobLayer->setFrame(rectForKnob());
    }
}

int
DSwitch::knobHeightForSize(QSize size)
{
    return size.height() - BORDER_LINE_WIDTH * 2.0;
}

QRect
DSwitch::rectForKnob()
{
    int height = knobHeightForSize(backgroundLayer->size());
    int width = 0;

    QRect bounds = backgroundLayer->getFrame();

    if (isActive) {
        width = (bounds.width() - 2 * BORDER_LINE_WIDTH) / DECREASED_GOLDEN_RATIO;
    }
    else {
        width = (bounds.width() - 2 * BORDER_LINE_WIDTH) / GOLDEN_RATIO;
    }

    int x = 0;
    if ((!hasDragged && !isOn) || (hasDragged && !isDraggingTowardsOn)) {
        x = BORDER_LINE_WIDTH;
    }
    else {
        x = bounds.width() - width - BORDER_LINE_WIDTH;
    }

    return QRect(x, BORDER_LINE_WIDTH, width, height);

}

void
DSwitch::resizeEvent(QResizeEvent *ev)
{
    backgroundLayer->resize(ev->size());
    reloadLayerSize(false);
}

bool
DSwitch::eventFilter(QObject *target, QEvent *ev)
{
    if (!enabled) return QWidget::eventFilter(target, ev);

    if (ev->type() == QEvent::MouseButtonPress) {
        isActive = true;
        reloadLayer(true);
        return true;
    }
    else if (ev->type() == QEvent::MouseButtonRelease) {
        isActive = false;
        QMouseEvent *me = (QMouseEvent *)ev;

        bool oldOn = isOn;
        if (hasDragged)
            isOn = isDraggingTowardsOn;
        else
            isOn = me->pos().x() > width() / 2;

        if (oldOn != isOn) 
            emit this->onValueChanged(isOn);

        hasDragged = false;
        isDraggingTowardsOn = false;
        reloadLayer(true);
        return true;
    }
    else if (ev->type() == QEvent::MouseMove) {
        hasDragged = true;
        QMouseEvent *me = (QMouseEvent *)ev;
        isDraggingTowardsOn = me->pos().x() > width() / 2;

        reloadLayer(true);

        return true;
    }

    return QWidget::eventFilter(target, ev);
}
