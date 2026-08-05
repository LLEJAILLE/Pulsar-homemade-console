#pragma once

#include <QWidget>

class BackgroundWidget;
class QLabel;
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class QResizeEvent;
class QVBoxLayout;

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateLogoPixmap();

    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_logoLabel = nullptr;
    QGraphicsOpacityEffect *m_logoOpacity = nullptr;
    QPropertyAnimation *m_fadeAnimation = nullptr;
    QPixmap m_logoPixmap;
};