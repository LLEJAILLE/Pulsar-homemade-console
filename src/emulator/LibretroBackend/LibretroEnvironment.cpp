#include "LibretroEnvironment.h"

#include "utils/Paths.h"

#include <QDir>
#include "Libretro.h"

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