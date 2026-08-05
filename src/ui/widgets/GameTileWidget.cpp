#include "GameTileWidget.h"

#include "utils/Paths.h"

#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

namespace
{
    constexpr int kCornerRadius = 24;
}

GameTileWidget::GameTileWidget(const Game& game, QWidget* parent) : QWidget(parent), m_game(game) {
    m_cover = loadCover(QString::fromStdString(m_game.title));
    setAttribute(Qt::WA_TranslucentBackground);

    m_shadow = new QGraphicsDropShadowEffect(this);

    m_shadow->setBlurRadius(50);
    m_shadow->setOffset(0);
    m_shadow->setColor(QColor(10, 10, 10, 180));

    setGraphicsEffect(m_shadow);

    m_shadow->setEnabled(false);

    setTileSize(m_tileSize);
    m_animation = new QPropertyAnimation(this, "yOffset");

    m_animation->setStartValue(-4);
    m_animation->setEndValue(4);

    m_animation->setDuration(400);

    m_animation->setEasingCurve(QEasingCurve::InOutSine);

    connect(m_animation, &QPropertyAnimation::finished, this, [this]()
    {
        if (m_animation->direction() == QAbstractAnimation::Forward)
            m_animation->setDirection(QAbstractAnimation::Backward);
        else
            m_animation->setDirection(QAbstractAnimation::Forward);

        m_animation->start();
    });
}

QSize GameTileWidget::sizeHint() const
{
    return QSize(m_tileSize + 30, m_tileSize);
}

void GameTileWidget::setTileSize(int size)
{
    m_tileSize = size;
    setFixedSize(size + 30, size);
    update();
}

QPixmap GameTileWidget::loadCover(const QString& gameName)
{
    QString path = QDir(Paths::icons()).filePath(gameName + QStringLiteral(".png"));

    if (!QFile::exists(path))
    {
        path = QDir(Paths::icons()).filePath(QStringLiteral("placeholder.png"));
    }

    QPixmap source(path);

    if (source.isNull())
        return {};

    QPixmap rounded(source.size());
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath clip;
    clip.addRoundedRect(
        rounded.rect(),
        kCornerRadius,
        kCornerRadius);

    painter.setClipPath(clip);
    painter.drawPixmap(0, 0, source);

    return rounded;
}

void GameTileWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect imageRect(15, 8 + m_yOffset, m_tileSize - 16, m_tileSize - 16);


    painter.drawPixmap(
        imageRect,
        m_cover.scaled(
            imageRect.size(),
            Qt::KeepAspectRatioByExpanding, 
            Qt::SmoothTransformation));

    if (m_selected) {
        painter.setClipping(false);

    }
}

void GameTileWidget::setSelected(bool selected)
{
    if (m_selected == selected)
        return;

    m_selected = selected;

    m_shadow->setEnabled(selected);

    if (selected)
    {
        m_animation->setDirection(QAbstractAnimation::Forward);
        m_animation->start();
    }
    else
    {
        m_animation->stop();
        setYOffset(0);
    }
}

void GameTileWidget::setYOffset(int value)
{
    m_yOffset = value;
    update();
}