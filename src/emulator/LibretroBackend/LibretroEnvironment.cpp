#include "LibretroEnvironment.h"

#include "utils/Paths.h"

#include <QDir>
#include <iostream>

#include "Libretro.h"

namespace
{
QByteArray legacyOptionDefault(const char* definition)
{
    const QByteArray values(definition);
    const int separator = values.indexOf(';');

    if (separator < 0)
        return {};

    const int valueStart = separator + 1;
    const int firstValue = values.indexOf('|', valueStart);
    return values.mid(valueStart, firstValue - valueStart).trimmed();
}
}

retro_pixel_format LibretroEnvironment::m_pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
QMap<QString, QByteArray> LibretroEnvironment::m_optionCache;

retro_pixel_format LibretroEnvironment::pixelFormat()
{
    return m_pixelFormat;
}

void LibretroEnvironment::setCoreOptions(const QMap<QString, QString>& options)
{
    m_optionCache.clear();

    for (auto it = options.cbegin(); it != options.cend(); ++it) {
        m_optionCache.insert(it.key(), it.value().toUtf8());
    }

    std::cout << "[libretro] Configured option overrides: "
              << m_optionCache.size() << std::endl;
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

    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    {
        static QByteArray saveDir;

        QDir().mkpath(Paths::saves());
        saveDir = QDir::toNativeSeparators(Paths::saves()).toLocal8Bit();
        *static_cast<const char**>(data) = saveDir.constData();
        std::cout << "[libretro] Save directory: " << saveDir.constData() << std::endl;
        return true;
    }

    case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
        const auto variables = static_cast<const retro_variable*>(data);
        int optionCount = 0;

        if (variables) {
            for (const retro_variable* variable = variables; variable->key; ++variable) {
                ++optionCount;
                const QString key = QString::fromUtf8(variable->key);

                if (!m_optionCache.contains(key) && variable->value) {
                    const QByteArray defaultValue = legacyOptionDefault(variable->value);
                    if (!defaultValue.isEmpty()) {
                        m_optionCache.insert(key, defaultValue);
                    }
                }

                const auto option = m_optionCache.constFind(key);
                std::cout << "[libretro] SET_VARIABLES: " << variable->key
                          << " = "
                          << (option == m_optionCache.cend() ? "(NOT SET)" : option->constData())
                          << std::endl;
            }
        }

        std::cout << "[libretro] SET_VARIABLES received: "
                  << optionCount << " definitions" << std::endl;
        return true;
    }

    case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
        if (!data)
            return false;

        auto var = static_cast<retro_variable*>(data);

        if (!var->key)
            return false;

        const QString key = QString::fromUtf8(var->key);
        const auto it = m_optionCache.constFind(key);

        if (it == m_optionCache.cend()) {
            var->value = nullptr;

            std::cout << "[libretro] GET_VARIABLE: "
                    << key.toStdString()
                    << " = (NOT SET)"
                    << std::endl;
            std::cout.flush();

            return true;
        }

        var->value = it.value().constData();

        std::cout << "[libretro] GET_VARIABLE: "
                << key.toStdString()
                << " = "
                << var->value
                << std::endl;
        std::cout.flush();

        return true;
    }

    default:
        std::cout << "[libretro] Unsupported environment command: "
                  << cmd << std::endl;
        return false;
    }
}