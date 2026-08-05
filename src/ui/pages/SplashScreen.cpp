#include "SplashScreen.h"

#include "../widgets/BackgroundWidget.h"

#include "utils/Paths.h"

#include <QGraphicsOpacityEffect>
#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace
{
constexpr int kLogoBaseSize = 360;
constexpr int kLogoMinSize = 180;
}

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("SplashScreen"));

    m_background = new BackgroundWidget(this);

    m_overlay = new QWidget(this);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);

    auto *layout = new QVBoxLayout(m_overlay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addStretch(1);

    m_logoLabel = new QLabel(m_overlay);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_logoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_logoOpacity = new QGraphicsOpacityEffect(m_logoLabel);
    m_logoOpacity->setOpacity(0.0);
    m_logoLabel->setGraphicsEffect(m_logoOpacity);

    m_logoPixmap = QPixmap(QDir(Paths::img()).filePath(QStringLiteral("logo.png")));
    updateLogoPixmap();

    layout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
    layout->addStretch(1);

    m_fadeAnimation = new QPropertyAnimation(m_logoOpacity, "opacity", this);
    m_fadeAnimation->setDuration(4000);
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();
}

void SplashScreen::updateLogoPixmap()
{
    if (!m_logoLabel) {
        return;
    }

    if (m_logoPixmap.isNull())
    {
        m_logoLabel->setText(QStringLiteral("PULSAR"));
        return;
    }

    const int targetSide = qMax(kLogoMinSize, qMin(width(), height()) / 2);
    const QSize targetSize(qMin(kLogoBaseSize, targetSide), qMin(kLogoBaseSize, targetSide));
    m_logoLabel->setPixmap(m_logoPixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void SplashScreen::resizeEvent(QResizeEvent *event)
{
    if (m_background != nullptr)
    {
        m_background->setGeometry(rect());
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setGeometry(rect());
    }

    updateLogoPixmap();

    QWidget::resizeEvent(event);
}