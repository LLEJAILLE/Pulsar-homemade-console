#include <QHash>
#include <QString>

#include "Libretro.h"

class LibretroEnvironment
{
    public:
        static bool callback(unsigned cmd, void* data);
        static retro_pixel_format pixelFormat();
        static void resetCoreOptions();
        static void setCoreOption(const QString& key, const QString& value);

    private:
        static retro_pixel_format m_pixelFormat;
        static QHash<QString, QString> m_requestedOptions;
        static QHash<QString, QString> m_availableOptions;
};