#include "BottomHomePage.h"

#include "../widgets/GameTileWidget.h"
#include "../../audio/AudioManager.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QPainter>
#include <QHideEvent>

#include <algorithm>

namespace
{
constexpr int kPageMargin = 16;
constexpr int kPageSpacing = 10;
constexpr int kTitleFontSize = 24;
constexpr int kTileSpacing = 24;

const char *kHomePageStyle = R"(
    HomePage {
        background-color: rgb(248, 0, 0)
    }

    QLabel {
        color: white;
    }

    QScrollArea {
        background: transparent;
        border-radius:18px;
    }

    QWidget#GamesContainer {
        background: transparent;
    }

    QLabel#HomePageTitle {
        background: transparent;
    }

    QLabel#HourLabel {
        background: transparent;
    }

)";
}

void BottomHomePage::updateTileSizes()
{
    if (!m_scrollArea || !m_scrollArea->viewport() || !m_gamesLayout || !m_container) {
        return;
    }

    const int viewportHeight = m_scrollArea->viewport()->height();
    if (viewportHeight <= 0) {
        return;
    }

    const int tileSide = viewportHeight - 3 * kPageMargin;
    const int sideMargin = std::max(0, (m_scrollArea->viewport()->width() - tileSide) / 2);

    m_gamesLayout->setContentsMargins(sideMargin, 0, sideMargin, 0);

    for (auto *tile : m_tiles)
    {
        if (!tile) {
            continue;
        }

        tile->setTileSize(tileSide);
        tile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        tile->setFocusPolicy(Qt::NoFocus);
    }

    m_container->adjustSize();
    m_container->updateGeometry();
    m_container->setMinimumHeight(tileSide);
}

void BottomHomePage::updateSelection()
{
    for (std::size_t index = 0; index < m_tiles.size(); ++index) {
        if (m_tiles[index]) {
            m_tiles[index]->setSelected(static_cast<int>(index) == m_currentIndex);
        }
    }
}

void BottomHomePage::centerCurrentTile()
{
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_tiles.size()) || !m_scrollArea) {
        return;
    }

    auto *tile = m_tiles[static_cast<std::size_t>(m_currentIndex)];
    if (!tile) {
        return;
    }

    const int center = tile->x() + tile->width() / 2 - m_scrollArea->viewport()->width() / 2;
    auto* bar = m_scrollArea->horizontalScrollBar();
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(bar->value());
    m_scrollAnimation->setEndValue(center);
    m_scrollAnimation->start();
}

void BottomHomePage::setCurrentIndex(int newIndex)
{
    if (m_tiles.empty()) {
        m_currentIndex = -1;
        updateSelection();
        return;
    }

    m_currentIndex = std::clamp(newIndex, 0, static_cast<int>(m_tiles.size()) - 1);
    updateSelection();

    if (m_currentIndex == m_settingsIndex) {
        emit selectedGameChanged(QStringLiteral("Settings"));
        return;
    }

    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_gameTitles.size())) {
        emit selectedGameChanged(m_gameTitles[static_cast<std::size_t>(m_currentIndex)]);
    }
}

void BottomHomePage::resizeEvent(QResizeEvent *event)
{
    m_background->setGeometry(rect());
    m_overlay->setGeometry(rect());

    QWidget::resizeEvent(event);
    updateTileSizes();
    centerCurrentTile();
    updateCurrentTime();
}

void BottomHomePage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    AudioManager::instance().playHomeMusic();

    QTimer::singleShot(0, this, [this]()
    {
        updateCurrentTime();
        updateTileSizes();
        centerCurrentTile();
        setFocus(Qt::OtherFocusReason);
    });
}

void BottomHomePage::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);

    AudioManager::instance().stopHomeMusic();
}

void BottomHomePage::keyPressEvent(QKeyEvent *event)
{
    if (m_tiles.empty()) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Right:
        if (m_currentIndex < static_cast<int>(m_tiles.size()) - 1) {
            audioManager.playSwitchMenuItem();
            setCurrentIndex(m_currentIndex + 1);
            centerCurrentTile();
        }
        event->accept();
        return;

    case Qt::Key_Left:
        if (m_currentIndex > 0) {
            audioManager.playSwitchMenuItem();
            setCurrentIndex(m_currentIndex - 1);
            centerCurrentTile();
        }
        event->accept();
        return;

    case Qt::Key_Space:
    case Qt::Key_A:
        if (m_currentIndex == m_settingsIndex) {
            emit openSettingsRequested();
        } else if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_games.size())) {
            emit launchGame(m_games[m_currentIndex]);
        }
        event->accept();
        return;

    default:
        QWidget::keyPressEvent(event);
        return;
    }
}

std::string BottomHomePage::currentTime() const
{
    char buffer[20];
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    #ifdef _WIN32
        localtime_s(&localTime, &now);
    #else
        localtime_r(&now, &localTime);
    #endif
    
    std::strftime(buffer, sizeof(buffer), "%d/%m %H:%M", &localTime);

    return std::string(buffer);
}

void BottomHomePage::updateCurrentTime()
{
    std::string newTimeString = currentTime();
    if (newTimeString != currentTimeString) {
        currentTimeString = newTimeString;
        if (m_hourLabel) {
            m_hourLabel->setText(QString::fromStdString(currentTimeString));
        }
    }
}

void BottomHomePage::updateBatteryLevel()
{
    if (!m_batteryWidget) {
        return;
    }

    m_batteryWidget->setBatteryLevel(m_batteryLevel);
}

BottomHomePage::BottomHomePage(const std::vector<Game> &games, QWidget *parent) : QWidget(parent), m_scrollArea(new QScrollArea(this)), m_container(new QWidget(m_scrollArea)), m_gamesLayout(new QHBoxLayout(m_container))
{
    audioManager.setVolume(0.2f);

    setObjectName(QStringLiteral("HomePage"));
    setStyleSheet(QString::fromLatin1(kHomePageStyle));
    setFocusPolicy(Qt::StrongFocus);

    m_games = games;
    m_container->setObjectName("GamesContainer");

    m_background = new BackgroundWidget(this);

    m_overlay = new QWidget(this);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QVBoxLayout(m_overlay);
    
    layout->setContentsMargins(kPageMargin, kPageMargin, kPageMargin, kPageMargin);
    layout->setSpacing(kPageSpacing);

    auto *titleLabel = new QLabel(QStringLiteral("Pulsar LAB"), this);
    auto *hourLabel = new QLabel(QString::fromStdString(currentTimeString), this);
    m_hourLabel = hourLabel;
    hourLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    m_batteryWidget = new BatteryWidget(this);
    m_batteryWidget->setBatteryLevel(m_batteryLevel);

    titleLabel->setObjectName("HomePageTitle");
    titleLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 700;").arg(kTitleFontSize));
    hourLabel->setObjectName("HourLabel");
    hourLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: 700;").arg(kTitleFontSize - 4));

    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_scrollArea->viewport()->setAutoFillBackground(false);
    m_container->setAutoFillBackground(false);
    m_scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    m_container->setAttribute(Qt::WA_TranslucentBackground);

    m_container->setObjectName(QStringLiteral("GamesContainer"));
    m_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_gamesLayout->setContentsMargins(0, 0, 0, 0);
    m_gamesLayout->setSpacing(kTileSpacing);

    m_scrollAnimation = new QPropertyAnimation(m_scrollArea->horizontalScrollBar(), "value", this);
    m_scrollAnimation->setDuration(250);

    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, &QTimer::timeout, this, &BottomHomePage::updateCurrentTime);
    m_timeTimer->start(1000);

    for (const Game &game : games) {
        m_gameTitles.push_back(QString::fromStdString(game.title));
        auto *tile = new GameTileWidget(game, m_container);
        m_tiles.push_back(tile);
        m_gamesLayout->addWidget(tile);
    }

    Game settingsGame;
    settingsGame.title = "settings";
    auto *settingsTile = new GameTileWidget(settingsGame, m_container);
    m_tiles.push_back(settingsTile);
    m_gamesLayout->addWidget(settingsTile);
    m_settingsIndex = static_cast<int>(m_tiles.size()) - 1;

    if (!m_tiles.empty()) {
        setCurrentIndex(0);
    }

    m_scrollArea->setWidget(m_container);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(hourLabel);
    headerLayout->addWidget(m_batteryWidget);

    layout->addLayout(headerLayout);
    layout->addWidget(m_scrollArea, 1);
}

