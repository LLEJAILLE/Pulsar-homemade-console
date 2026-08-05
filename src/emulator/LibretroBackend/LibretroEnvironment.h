#include <QString>

#include "Libretro.h"

class LibretroEnvironment
{
    public:
        static bool callback(unsigned cmd, void* data);
        static retro_pixel_format pixelFormat();

    private:
        static QString systemDirectory;
        static QString saveDirectory;
        static retro_pixel_format m_pixelFormat;
};