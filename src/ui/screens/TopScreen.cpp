#include "TopScreen.h"

#include "../pages/TopHomePage.h"
#include "../pages/SplashScreen.h"
#include "../../utils/env.hpp"

#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

namespace
{
    const QColor kBackgroundColor("#303030");
    const QColor kBorderColor("#5a5a5a");
    const QColor kTextColor("#f0f0f0");
}

TopScreen::TopScreen(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(SCREEN_SIZE_X, SCREEN_SIZE_Y);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
}

void TopScreen::loadPage(Page page)
{
    if (m_currentPage == page && m_pageWidget != nullptr) {
        return;
    }

    m_currentPage = page;

    if (m_pageWidget != nullptr) {
        m_layout->removeWidget(m_pageWidget);
        delete m_pageWidget;
        m_pageWidget = nullptr;
    }

    if (page == Page::SplashScreen) {
        m_pageWidget = new SplashScreen(this);
    } else if (page == Page::Home) {
        m_pageWidget = new TopHomePage(m_gameTitle, this);
    } else {
        auto *placeholder = new QLabel(QStringLiteral("Page not implemented yet"), this);
        placeholder->setAlignment(Qt::AlignCenter);
        m_pageWidget = placeholder;
    }

    m_layout->addWidget(m_pageWidget);
}

void TopScreen::setGameTitle(const QString &gameTitle)
{
    if (m_gameTitle == gameTitle) {
        return;
    }

    m_gameTitle = gameTitle;

    if (auto *homePage = qobject_cast<TopHomePage *>(m_pageWidget)) {
        homePage->setGameTitle(m_gameTitle);
    }

    update();
}

void TopScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), kBackgroundColor);
    painter.setPen(QPen(kBorderColor, 2));
    painter.drawRect(rect().adjusted(1, 1, -2, -2));

    painter.setPen(kTextColor);
    painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("TOP SCREEN"));
}
