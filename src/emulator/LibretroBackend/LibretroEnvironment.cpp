#include "LibretroEnvironment.h"

#include "utils/Paths.h"

#include <QDir>
#include <iostream>

#include "Libretro.h"

retro_pixel_format LibretroEnvironment::m_pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
QMap<QString, QByteArray> LibretroEnvironment::m_optionCache;

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
        static QByteArray systemDir;

        systemDir = QDir::toNativeSeparators(Paths::system()).toLocal8Bit();
        *static_cast<const char**>(data) = systemDir.constData();
        return true;
    }

    case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
        if (!data)
            return false;

        auto var = static_cast<retro_variable*>(data);

        if (!var->key)
            return false;

        QString key = QString::fromUtf8(var->key);
        auto it = m_optionCache.find(key);

        if (it == m_optionCache.end()) {
            var->value = nullptr;

            std::cerr << "GET_VARIABLE: "
                    << key.toStdString()
                    << " = (NOT SET)"
                    << std::endl;
            std::cerr.flush();

            return true;
        }

        var->value = it.value().constData();

        std::cerr << "GET_VARIABLE: "
                << key.toStdString()
                << " = "
                << var->value
                << std::endl;
        std::cerr.flush();

        return true;
    }

    default:
        return false;
    }
}