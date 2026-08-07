#include "EmulatorScreenWidget.h"
#include "../../emulator/LibretroBackend/LibretroVideo.h"
#include "../../input/InputManager.h"
#include "../../utils/FrameTimingProfiler.h"

#include <algorithm>
#include <QPainter>
#include <QMouseEvent>
#include <QRect>

namespace
{
QPoint mapToContentPixels(const QWidget *widget, const QPointF &position, int screenIndex, int screenCount)
{
    const QImage &frame = LibretroVideo::frame();
    const int contentWidth = frame.isNull() ? 256 : frame.width();
    const int contentHeight = frame.isNull() ? 192 * std::max(1, screenCount) : frame.height();
    const int screenHeight = contentHeight / std::max(1, screenCount);
    const int screenOffset = screenIndex * screenHeight;

    const double mappedX = position.x() * (contentWidth - 1) / std::max(1, widget->width() - 1);
    const double mappedY = screenOffset + position.y() * (screenHeight - 1) / std::max(1, widget->height() - 1);

    return QPoint(qRound(mappedX), qRound(mappedY));
}
}

EmulatorScreenWidget::EmulatorScreenWidget(const ConsoleProfile& profile, int screenIndex, QWidget* parent)
    : QWidget(parent)
    , m_profile(profile)
    , m_screenIndex(screenIndex)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (m_profile.hasTouch() && m_screenIndex == m_profile.touchScreenIndex()) {
        setMouseTracking(true);
    }
}

bool EmulatorScreenWidget::isActiveScreen() const
{
    return m_screenIndex >= 0;
}

void EmulatorScreenWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (!isActiveScreen()) {
        FrameTimingProfiler::ScopedTimer renderingTimer(FrameTimingProfiler::Stage::Rendering);
        painter.fillRect(rect(), Qt::black);
        FrameTimingProfiler::notePaintCompleted();
        return;
    }

    QImage screen;
    {
        FrameTimingProfiler::ScopedTimer framebufferTimer(FrameTimingProfiler::Stage::Framebuffer);
        screen = LibretroVideo::screenForIndex(m_screenIndex, m_profile.screenCount());
    }

    {
        FrameTimingProfiler::ScopedTimer renderingTimer(FrameTimingProfiler::Stage::Rendering);
        painter.fillRect(rect(), Qt::black);

        if (!screen.isNull()) {
            if (m_profile.screenCount() <= 1) {
                QSize targetSize = screen.size();
                targetSize.scale(size(), Qt::KeepAspectRatio);

                QRect target(QPoint(0, 0), targetSize);
                target.moveCenter(rect().center());

                painter.drawImage(target, screen);
            } else {
                painter.drawImage(rect(), screen);
            }
        }
    }

    FrameTimingProfiler::notePaintCompleted();
}

void EmulatorScreenWidget::mousePressEvent(QMouseEvent* event)
{
    if (isActiveScreen() && m_profile.hasTouch() && m_screenIndex == m_profile.touchScreenIndex()) {
        const QPoint contentPoint = mapToContentPixels(this, event->position(), m_screenIndex, m_profile.screenCount());

        InputManager::pressTouch(contentPoint.x(), contentPoint.y());
    }

    QWidget::mousePressEvent(event);
}

void EmulatorScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (isActiveScreen() && m_profile.hasTouch() && m_screenIndex == m_profile.touchScreenIndex()) {
        const QPoint contentPoint = mapToContentPixels(this, event->position(), m_screenIndex, m_profile.screenCount());

        InputManager::moveTouch(contentPoint.x(), contentPoint.y());
    }

    QWidget::mouseMoveEvent(event);
}

void EmulatorScreenWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (isActiveScreen() && m_profile.hasTouch() && m_screenIndex == m_profile.touchScreenIndex())
        InputManager::releaseTouch();

    QWidget::mouseReleaseEvent(event);
}