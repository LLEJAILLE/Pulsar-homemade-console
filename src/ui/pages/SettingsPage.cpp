#include "SettingsPage.h"

#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>

#include <vector>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("SettingsPage"));
    setFocusPolicy(Qt::StrongFocus);

    m_background = new BackgroundWidget(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);

    auto *titleLabel = new QLabel(QStringLiteral("Parametre"), this);
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setStyleSheet(QStringLiteral("color: white; font-size: 28px; font-weight: 700;"));
    layout->addWidget(titleLabel);

    const std::vector<QString> settingNames = {
        QStringLiteral("Installer un nouveau jeu"),
        QStringLiteral("Volume"),
        QStringLiteral("Luminosite"),
    };

    layout->addSpacing(20);
    for (const QString &settingName : settingNames) {
        auto *row = new QLabel(settingName, this);
        row->setMinimumHeight(42);
        row->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_rows.push_back(row);
        layout->addWidget(row);
    }
    layout->addStretch(1);
    updateSelection();
}

void SettingsPage::updateSelection()
{
    for (int index = 0; index < static_cast<int>(m_rows.size()); ++index) {
        m_rows[static_cast<std::size_t>(index)]->setStyleSheet(
            index == m_selectedIndex
                ? QStringLiteral("color: white; font-size: 20px; font-weight: 700; background-color: rgba(255, 255, 255, 45); padding: 8px;")
                : QStringLiteral("color: white; font-size: 20px; padding: 8px;"));
    }
}

void SettingsPage::resizeEvent(QResizeEvent *event)
{
    m_background->setGeometry(rect());
    QWidget::resizeEvent(event);
}

void SettingsPage::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        const int direction = event->key() == Qt::Key_Down ? 1 : -1;
        m_selectedIndex = (m_selectedIndex + direction + static_cast<int>(m_rows.size()))
            % static_cast<int>(m_rows.size());
        updateSelection();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_A || event->key() == Qt::Key_Space ||
        event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_selectedIndex == 0) {
            emit openGameInstaller();
        }
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_B || event->key() == Qt::Key_Escape) {
        emit backToHome();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}