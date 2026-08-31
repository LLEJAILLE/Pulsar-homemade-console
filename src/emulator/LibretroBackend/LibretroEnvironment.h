#include <QByteArray>
#include <QMap>
#include <QString>

#include "Libretro.h"

class LibretroEnvironment
{
    public:
        static bool callback(unsigned cmd, void* data);
        static retro_pixel_format pixelFormat();

    private:
        static retro_pixel_format m_pixelFormat;
        static QMap<QString, QByteArray> m_optionCache;
};