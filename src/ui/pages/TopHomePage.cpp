#include "TopHomePage.h"

#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace
{
constexpr int kPageMargin = 8;  // Reduced for smaller screen
constexpr int kTitleFontSize = 18;  // Reduced for 240px height

const char *kHomePageStyle = R"(
    TopHomePage {
        background-color: rgb(248, 0, 0)
    }

    QLabel {
        color: white;
        background: transparent;
    }

    QLabel#TopHomePageTitle {
        background: transparent;
    }

    QLabel#TopHomePageGameTitle {
        background: transparent;
    }
)";
}

TopHomePage::TopHomePage(const QString &gameTitle, QWidget *parent)
    : QWidget(parent)
    , m_gameTitle(gameTitle)
{
    setObjectName(QStringLiteral("TopHomePage"));
    setStyleSheet(QString::fromLatin1(kHomePageStyle));

    m_background = new BackgroundWidget(this);
    m_overlay = new QWidget(this);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);

    auto *layout = new QVBoxLayout(m_overlay);
    layout->setContentsMargins(kPageMargin, kPageMargin, kPageMargin, kPageMargin);
    layout->setSpacing(0);

    m_titleLabel = new QLabel(QStringLiteral("PULSAR HOMEMADE CONSOLE"), m_overlay);
    m_titleLabel->setObjectName(QStringLiteral("TopHomePageTitle"));
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 700;").arg(kTitleFontSize));
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_gameTitleLabel = new QLabel(m_gameTitle, m_overlay);
    m_gameTitleLabel->setObjectName(QStringLiteral("TopHomePageGameTitle"));
    m_gameTitleLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 700;").arg(kTitleFontSize - 4));
    m_gameTitleLabel->setAlignment(Qt::AlignCenter);
    m_gameTitleLabel->setWordWrap(true);

    layout->addWidget(m_titleLabel);
    layout->addStretch(1);
    layout->addWidget(m_gameTitleLabel);
}

void TopHomePage::setGameTitle(const QString &gameTitle)
{
    if (m_gameTitle == gameTitle)
    {
        return;
    }

    m_gameTitle = gameTitle;

    if (m_gameTitleLabel != nullptr)
    {
        m_gameTitleLabel->setText(m_gameTitle);
    }
}

void TopHomePage::resizeEvent(QResizeEvent *event)
{
    if (m_background != nullptr)
    {
        m_background->setGeometry(rect());
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setGeometry(rect());
    }

    QWidget::resizeEvent(event);
}