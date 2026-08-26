#include "LibretroEnvironment.h"

#include "utils/Paths.h"

#include <QDir>
#include "Libretro.h"

#include <cstdarg>
#include <cstdio>

namespace
{
void logCallback(retro_log_level, const char* format, ...)
{
    if (!format)
        return;

    va_list arguments;
    va_start(arguments, format);
    std::vfprintf(stderr, format, arguments);
    va_end(arguments);
}
}

retro_pixel_format LibretroEnvironment::m_pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
QHash<QString, QString> LibretroEnvironment::m_requestedOptions;
QHash<QString, QString> LibretroEnvironment::m_availableOptions;

void LibretroEnvironment::resetCoreOptions()
{
    m_requestedOptions.clear();
    m_availableOptions.clear();
}

void LibretroEnvironment::setCoreOption(const QString& key, const QString& value)
{
    m_requestedOptions.insert(key, value);
}

retro_pixel_format LibretroEnvironment::pixelFormat()
{
    return m_pixelFormat;
}

bool LibretroEnvironment::callback(unsigned cmd, void* data)
{
    switch (cmd) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
    {
        auto* callback = static_cast<retro_log_callback*>(data);
        if (!callback)
            return false;

        callback->log = logCallback;
        return true;
    }

    case RETRO_ENVIRONMENT_SET_MESSAGE:
        return data != nullptr;

    case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
        m_availableOptions.clear();

        const auto* options = static_cast<const retro_variable*>(data);
        if (!options)
            return true;

        for (const retro_variable* option = options; option->key && option->value; ++option)
            m_availableOptions.insert(QString::fromUtf8(option->key), QString::fromUtf8(option->value));

        return true;
    }

    case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
        auto* variable = static_cast<retro_variable*>(data);
        if (!variable || !variable->key)
            return true;

        const QString key = QString::fromUtf8(variable->key);
        const auto definition = m_availableOptions.constFind(key);
        if (definition == m_availableOptions.constEnd())
            return true;

        QString value = definition.value();
        const int separator = value.indexOf(QLatin1Char(';'));
        if (separator >= 0)
            value = value.mid(separator + 1).trimmed().section(QLatin1Char('|'), 0, 0);

        const auto requested = m_requestedOptions.constFind(key);
        if (separator >= 0 && requested != m_requestedOptions.constEnd()) {
            const QString values = definition.value().mid(separator + 1);
            if (values.split(QLatin1Char('|')).contains(requested.value()))
                value = requested.value();
        }

        static QHash<QString, QByteArray> optionValues;
        optionValues.insert(key, value.toUtf8());
        variable->value = optionValues[key].constData();
        return true;
    }

    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    {
        static QByteArray saveDirectory;
        saveDirectory = QDir::toNativeSeparators(Paths::saves()).toLocal8Bit();
        if (!data)
            return false;

        *static_cast<const char**>(data) = saveDirectory.constData();
        return true;
    }

    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
    {
        if (!data)
            return false;

        *static_cast<unsigned*>(data) = 1;
        return true;
    }

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
    {
        const auto* options = static_cast<const retro_core_option_definition*>(data);
        m_availableOptions.clear();
        if (!options)
            return true;

        for (const auto* option = options; option->key; ++option) {
            QString definition = QString::fromUtf8(option->desc ? option->desc : option->key);
            definition += QStringLiteral("; ");

            bool firstValue = true;
            for (const auto& value : option->values) {
                if (!value.value)
                    break;

                if (!firstValue)
                    definition += QStringLiteral("|");
                definition += QString::fromUtf8(value.value);
                firstValue = false;
            }

            m_availableOptions.insert(QString::fromUtf8(option->key), definition);
        }

        return true;
    }

    case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
    {
        if (!data)
            return false;

        *static_cast<unsigned*>(data) = 0;
        return true;
    }

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

    default:
        return false;
    }
}