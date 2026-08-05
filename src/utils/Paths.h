#pragma once

#include <QString>

class Paths
{
public:
    static QString projectRoot();

    static QString assets();
    static QString audio();
    static QString backgrounds();
    static QString icons();
    static QString img();

    static QString cores();
    static QString bios();
    static QString system();

    static QString library();
    static QString roms();
    static QString saves();
    static QString config();

    static QString userDataRoot();
    static QString userConfig();
    static QString userSaves();
    static QString cache();

private:
    static QString join(const QString& base, const QString& relative);
};
