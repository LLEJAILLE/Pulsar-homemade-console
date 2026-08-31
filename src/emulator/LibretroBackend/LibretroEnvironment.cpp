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

void LibretroEnvironment::setOptions(const QMap<QString, QString>& options)
{
    m_optionCache.clear();

    for (auto it = options.cbegin(); it != options.cend(); ++it) {
        m_optionCache.insert(it.key(), it.value().toUtf8());
    }
}

void LibretroEnvironment::registerCoreOptions(const retro_core_options_v2* options)
{
    if (!options || !options->definitions)
        return;

    int optionCount = 0;

    for (const retro_core_option_v2_definition* definition = options->definitions;
         definition->key;
         ++definition) {
        ++optionCount;

        const QString key = QString::fromUtf8(definition->key);
        if (!m_optionCache.contains(key) && definition->default_value) {
            m_optionCache.insert(key, QByteArray(definition->default_value));
        }
    }

    std::cout << "Core options V2 registered: " << optionCount << std::endl;
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

    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
    {
        if (!data)
            return false;

        *static_cast<unsigned*>(data) = 2;
        return true;
    }

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
        registerCoreOptions(static_cast<const retro_core_options_v2*>(data));
        return true;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
    {
        const auto options = static_cast<const retro_core_options_v2_intl*>(data);
        registerCoreOptions(options ? options->us : nullptr);
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

            std::cout << "GET_VARIABLE: "
                    << key.toStdString()
                    << " = (NOT SET)"
                    << std::endl;
            std::cout.flush();

            return true;
        }

        var->value = it.value().constData();

        std::cout << "GET_VARIABLE: "
                << key.toStdString()
                << " = "
                << var->value
                << std::endl;
        std::cout.flush();

        return true;
    }

    default:
        return false;
    }
}