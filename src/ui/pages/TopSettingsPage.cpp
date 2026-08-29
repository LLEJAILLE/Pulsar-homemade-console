#include "TopSettingsPage.h"

#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace
{
constexpr int kPageMargin = 8;  // Reduced for smaller screen
constexpr int kTitleFontSize = 16;  // Reduced for 240px height

const char *kSettingsPageStyle = R"(
    TopSettingsPage {
        background-color: rgb(248, 0, 0)
    }

    QLabel {
        color: white;
        background: transparent;
    }
)";
}

TopSettingsPage::TopSettingsPage(const QString &statusMessage, QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("TopSettingsPage"));
    setStyleSheet(QString::fromLatin1(kSettingsPageStyle));

    m_background = new BackgroundWidget(this);
    m_overlay = new QWidget(this);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);

    auto *layout = new QVBoxLayout(m_overlay);
    layout->setContentsMargins(kPageMargin, kPageMargin, kPageMargin, kPageMargin);
    layout->setSpacing(6);

    m_titleLabel = new QLabel(QStringLiteral("SETTINGS - ROM IMPORT"), m_overlay);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 700;").arg(kTitleFontSize));

    m_statusLabel = new QLabel(statusMessage, m_overlay);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 600;").arg(kTitleFontSize - 6));

    const QString controls = QStringLiteral("A/Enter: open or install   |   B/Backspace: parent   |   Q/E: history   |   Esc: home");
    auto *controlsLabel = new QLabel(controls, m_overlay);
    controlsLabel->setWordWrap(true);
    controlsLabel->setStyleSheet(QStringLiteral("font-size: %1px;").arg(kTitleFontSize - 10));

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_statusLabel);
    layout->addStretch(1);
    layout->addWidget(controlsLabel);
}

void TopSettingsPage::setStatusMessage(const QString &statusMessage)
{
    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(statusMessage);
    }
}

void TopSettingsPage::resizeEvent(QResizeEvent *event)
{
    if (m_background != nullptr) {
        m_background->setGeometry(rect());
    }

    if (m_overlay != nullptr) {
        m_overlay->setGeometry(rect());
    }

    QWidget::resizeEvent(event);
}
