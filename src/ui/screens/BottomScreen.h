#pragma once

#include <vector>

#include <QPaintEvent>
#include <QWidget>

#include "library/Game.h"
#include "ui/pages/pages.hpp"

class BottomScreen : public QWidget
{
public:
    explicit BottomScreen(QWidget *parent = nullptr);

    void loadPage(Page page, const std::vector<Game> &games);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QWidget *m_pageWidget = nullptr;
    Page m_currentPage = Page::Home;
};