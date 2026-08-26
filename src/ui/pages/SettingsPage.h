#pragma once

#include "../widgets/BackgroundWidget.h"

#include <QWidget>

#include <vector>

class QKeyEvent;
class QResizeEvent;
class QLabel;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

signals:
    void backToHome();
    void openGameInstaller();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateSelection();

    BackgroundWidget *m_background = nullptr;
    std::vector<QLabel *> m_rows;
    int m_selectedIndex = 0;
};