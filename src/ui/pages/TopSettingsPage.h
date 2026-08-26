#pragma once

#include <QString>
#include <QWidget>

#include "../widgets/BackgroundWidget.h"

class QLabel;
class QResizeEvent;

class TopSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit TopSettingsPage(const QString &statusMessage = QString(), QWidget *parent = nullptr);

    void setStatusMessage(const QString &statusMessage);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
};
