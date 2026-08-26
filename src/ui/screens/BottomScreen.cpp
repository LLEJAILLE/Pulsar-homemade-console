#include "BottomScreen.h"

#include <QLabel>
#include <QVBoxLayout>

#include "../pages/SplashScreen.h"
#include "../pages/BottomHomePage.h"
#include "../pages/BottomSettingsPage.h"
#include "../../utils/env.hpp"
#include <QPainter>

namespace
{
    const QColor kBackgroundColor("#303030");
    const QColor kBorderColor("#5a5a5a");
    const QColor kTextColor("#f0f0f0");
}

BottomScreen::BottomScreen(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(SCREEN_SIZE_X, SCREEN_SIZE_Y);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

void BottomScreen::loadPage(Page page, const std::vector<Game> &games)
{
    if (m_currentPage == page && m_pageWidget != nullptr) {
        return;
    }

    m_currentPage = page;

    if (m_pageWidget != nullptr) {
        layout()->removeWidget(m_pageWidget);
        delete m_pageWidget;
        m_pageWidget = nullptr;
    }

    if (page == Page::SplashScreen) {
        m_pageWidget = new SplashScreen(this);
    }
    else if (page == Page::Home) {
        m_pageWidget = new BottomHomePage(games, this);
    } else if (page == Page::Settings) {
        m_pageWidget = new BottomSettingsPage(this);
    } else {
        auto *placeholder = new QLabel(QStringLiteral("Page not implemented yet"), this);
        placeholder->setAlignment(Qt::AlignCenter);
        m_pageWidget = placeholder;
    }

    layout()->addWidget(m_pageWidget);
}

void BottomScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), kBackgroundColor);
    painter.setPen(QPen(kBorderColor, 2));
    painter.drawRect(rect().adjusted(1, 1, -2, -2));

    painter.setPen(kTextColor);
    painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("BOTTOM SCREEN"));
}
