#pragma once

#include <QPaintEvent>
#include <QWidget>

#include <QString>

#include "ui/pages/pages.hpp"

class QVBoxLayout;

class TopScreen : public QWidget
{
public:
    explicit TopScreen(QWidget *parent = nullptr);

    void loadPage(Page page);
    void setGameTitle(const QString &gameTitle);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *m_layout = nullptr;
    Page m_currentPage = Page::Home;
    QString m_gameTitle;
    QWidget *m_pageWidget = nullptr;
};