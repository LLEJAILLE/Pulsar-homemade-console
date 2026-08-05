#include "LibretroEnvironment.h"

#include <QCoreApplication>
#include <QDir>
#include "Libretro.h"

QString LibretroEnvironment::systemDirectory;
QString LibretroEnvironment::saveDirectory;
retro_pixel_format LibretroEnvironment::m_pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;

retro_pixel_format LibretroEnvironment::pixelFormat()
{
    return m_pixelFormat;
}

bool LibretroEnvironment::callback(unsigned cmd, void* data)
{
    switch (cmd) {
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: 
    {
        auto format = static_cast<retro_pixel_format*>(data);

        if (*format == RETRO_PIXEL_FORMAT_XRGB8888 || *format == RETRO_PIXEL_FORMAT_RGB565) {
            m_pixelFormat = *format;
            return true;
        }

        return false;
    }

    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
        if (systemDirectory.isEmpty()) {
            systemDirectory =
                QCoreApplication::applicationDirPath() + "/system";
        }

        static QByteArray systemDir;

        systemDir = QDir::toNativeSeparators(systemDirectory).toLocal8Bit();
        *static_cast<const char**>(data) = systemDir.constData();
        return true;
    }

    default:
        return false;
    }
}