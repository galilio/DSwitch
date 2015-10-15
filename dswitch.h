#ifndef __dswitch_h__
#define __dswitch_h__

#include <QWidget>
#include <QRect>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QAnimationGroup>
#include <QDebug>
#include <QEvent>

class DLayer : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QRect frame READ getFrame WRITE setFrame)
    Q_PROPERTY(int cornerRadius READ getCornerRadius WRITE setCornerRadius)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(int opacity READ getOpacity WRITE setOpacity)

private:
    int mCornerRadius;
    QColor mColor;
    qreal mOpacity;

    int mBorderWidth;

public:
    DLayer(QWidget *parent = 0);
    ~DLayer();

    QRect getFrame() {return geometry();}
    void setFrame(QRect frame) {setGeometry(frame);update();}

    int getCornerRadius() {return mCornerRadius;}
    void setCornerRadius(int cornerRadius) {mCornerRadius = cornerRadius; update();}

    QColor getColor() {return mColor;}
    void setColor(QColor color) {mColor = color; update();}

    qreal getOpacity() {return mOpacity;}
    void setOpacity(qreal opacity) {mOpacity = opacity;}

    void setBorderWidth(int width) {mBorderWidth = width;}

protected:
    virtual void paintEvent(QPaintEvent * event);
};

class DSwitch : public QWidget {
    Q_OBJECT

signals:
    void onValueChanged(bool on);

public:
    DSwitch(QWidget *parent = 0);
    ~DSwitch();

    bool isOn;
    QColor tintColor;
    bool enabled;

    bool isActive;
    bool hasDragged;
    bool isDraggingTowardsOn;

    DLayer *backgroundLayer;
    DLayer *knobLayer;

private:
    void setup();
    void setupLayers();

    void reloadLayerSize(bool anim, QAnimationGroup *group = NULL);

    void reloadLayer(bool anim, QAnimationGroup *group = NULL);

    int knobHeightForSize(QSize size);
    QRect rectForKnob();

protected:
    bool eventFilter(QObject *target, QEvent *ev);
    void resizeEvent(QResizeEvent *ev);
};

#endif