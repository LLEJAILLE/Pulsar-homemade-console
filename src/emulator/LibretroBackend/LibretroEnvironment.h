#include <QByteArray>
#include <QMap>
#include <QString>

#include "Libretro.h"

class LibretroEnvironment
{
    public:
        static bool callback(unsigned cmd, void* data);
        static retro_pixel_format pixelFormat();
        static void setOptions(const QMap<QString, QString>& options);

    private:
        static void registerCoreOptions(const retro_core_options_v2* options);

        static retro_pixel_format m_pixelFormat;
        static QMap<QString, QByteArray> m_optionCache;
};