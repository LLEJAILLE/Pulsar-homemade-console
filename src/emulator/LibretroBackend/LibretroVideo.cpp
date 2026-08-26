#include "LibretroVideo.h"

#include "LibretroEnvironment.h"

#include <algorithm>
#include <cstring>

QImage LibretroVideo::m_frame;

void LibretroVideo::videoRefresh(const void* data, unsigned width, unsigned height, size_t pitch)
{
    if (!data)
        return;

    const QImage::Format imageFormat =
        (LibretroEnvironment::pixelFormat() == RETRO_PIXEL_FORMAT_RGB565)
            ? QImage::Format_RGB16
            : QImage::Format_RGB32;

    if (m_frame.size() != QSize(width, height) || m_frame.format() != imageFormat) {
        m_frame = QImage(width, height, imageFormat);
    }

    const auto *source = static_cast<const unsigned char*>(data);
    unsigned char *destination = m_frame.bits();
    const int destinationStride = m_frame.bytesPerLine();
    const size_t copyBytes = std::min(pitch, static_cast<size_t>(destinationStride));

    for (unsigned row = 0; row < height; ++row) {
        std::memcpy(destination + static_cast<size_t>(row) * destinationStride,
                    source + static_cast<size_t>(row) * pitch,
                    copyBytes);

        if (imageFormat == QImage::Format_RGB32) {
            auto *pixels = reinterpret_cast<quint32*>(destination + static_cast<size_t>(row) * destinationStride);
            for (unsigned column = 0; column < width; ++column) {
                pixels[column] |= 0xff000000u;
            }
        }
    }

}

const QImage& LibretroVideo::frame()
{
    return m_frame;
}

QRect LibretroVideo::sourceRectForIndex(int screenIndex, int screenCount)
{
    if (m_frame.isNull())
        return {};

    if (screenCount <= 1)
        return m_frame.rect();

    const int clampedScreenCount = std::max(1, screenCount);
    const int clampedScreenIndex = std::clamp(screenIndex, 0, clampedScreenCount - 1);
    const int screenHeight = m_frame.height() / clampedScreenCount;
    return QRect(0, clampedScreenIndex * screenHeight, m_frame.width(), screenHeight);
}

QImage LibretroVideo::screenForIndex(int screenIndex, int screenCount)
{
    const QRect sourceRect = sourceRectForIndex(screenIndex, screenCount);
    return sourceRect.isValid() ? m_frame.copy(sourceRect) : QImage();
}

QImage LibretroVideo::topScreen()
{
    return screenForIndex(0, 2);
}

QImage LibretroVideo::bottomScreen()
{
    return screenForIndex(1, 2);
}