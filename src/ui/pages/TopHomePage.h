#pragma once

#include <QString>
#include <QWidget>

#include "../widgets/BackgroundWidget.h"

class QLabel;
class QResizeEvent;
class QVBoxLayout;

class TopHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit TopHomePage(const QString &gameTitle = QString(), QWidget *parent = nullptr);

    void setGameTitle(const QString &gameTitle);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString m_gameTitle;
    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_gameTitleLabel = nullptr;
};