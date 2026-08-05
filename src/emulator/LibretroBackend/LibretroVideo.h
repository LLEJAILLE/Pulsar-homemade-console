#pragma once

#include <QImage>

class LibretroVideo
{
    public:
        static void videoRefresh(const void* data, unsigned width, unsigned height, size_t pitch);

        static const QImage& frame();

        static QImage screenForIndex(int screenIndex, int screenCount);

        static QImage topScreen();
        static QImage bottomScreen();

    private:
        static QImage m_frame;

};