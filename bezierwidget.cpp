#include "bezierwidget.h"

#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QtGlobal>
#include <limits>

namespace {
QColor polygonColorForIndex(int index)
{
    return (index % 2 == 0) ? QColor(30, 30, 30) : QColor(170, 40, 40);
}

QColor curveColorForIndex(int index)
{
    return (index % 2 == 0) ? QColor(20, 150, 70) : QColor(45, 85, 220);
}

QString digitSubscript(int value)
{
    static const QString subs[10] = {
        QString::fromUtf8("₀"), QString::fromUtf8("₁"), QString::fromUtf8("₂"),
        QString::fromUtf8("₃"), QString::fromUtf8("₄"), QString::fromUtf8("₅"),
        QString::fromUtf8("₆"), QString::fromUtf8("₇"), QString::fromUtf8("₈"),
        QString::fromUtf8("₉")
    };

    if (value >= 0 && value <= 9)
        return subs[value];
    return QString::number(value);
}
}

BezierWidget::BezierWidget(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setToolTip(QString::fromUtf8(
        "ЛКМ — добавить/переместить; Shift+ЛКМ — вставить между точками; "
        "ПКМ — удалить; Delete — очистить; H — многоугольники; "
        "N — подписи; C — построение де Кастельжо"));
}

void BezierWidget::clear()
{
    m_points.clear();
    m_draggedIndex = -1;
    m_dragOffset = QPointF();
    update();
}

QVector<BezierWidget::Segment> BezierWidget::buildSegments() const
{
    QVector<Segment> segments;
    const int n = m_points.size();

    if (n < 2)
        return segments;

    if (n == 2) {
        const QPointF a = m_points[0];
        const QPointF b = m_points[1];
        segments.append(Segment{a, a + (b - a) / 3.0,
                                a + (b - a) * (2.0 / 3.0), b});
        return segments;
    }

    if (n == 3) {
        const QPointF a = m_points[0];
        const QPointF b = m_points[1];
        const QPointF c = m_points[2];
        segments.append(Segment{a, a + (b - a) * (2.0 / 3.0),
                                c + (b - c) * (2.0 / 3.0), c});
        return segments;
    }

    if (n == 4) {
        segments.append(Segment{m_points[0], m_points[1], m_points[2], m_points[3]});
        return segments;
    }

    QPointF joint = (m_points[2] + m_points[3]) / 2.0;
    segments.append(Segment{m_points[0], m_points[1], m_points[2], joint});

    for (int i = 3; i < n; i += 2) {
        if (i + 2 == n - 1) {
            segments.append(Segment{joint, m_points[i], m_points[i + 1], m_points[i + 2]});
            break;
        }

        if (i + 1 == n - 1) {
            const QPointF p = m_points[i];
            const QPointF last = m_points[i + 1];
            segments.append(Segment{joint, p, (p + last) / 2.0, last});
            break;
        }

        const QPointF nextJoint = (m_points[i + 1] + m_points[i + 2]) / 2.0;
        segments.append(Segment{joint, m_points[i], m_points[i + 1], nextJoint});
        joint = nextJoint;
    }

    return segments;
}

int BezierWidget::pointAt(const QPointF &position) const
{
    for (int i = m_points.size() - 1; i >= 0; --i) {
        if (QLineF(position, m_points[i]).length() <= 12.0)
            return i;
    }
    return -1;
}

int BezierWidget::nearestEdge(const QPointF &position) const
{
    int nearest = -1;
    double minimum = std::numeric_limits<double>::max();

    for (int i = 0; i + 1 < m_points.size(); ++i) {
        const QPointF start = m_points[i];
        const QPointF direction = m_points[i + 1] - start;
        const double squaredLength = QPointF::dotProduct(direction, direction);
        const double projection = squaredLength > 0.0
                                      ? qBound(0.0, QPointF::dotProduct(position - start, direction) / squaredLength, 1.0)
                                      : 0.0;
        const QPointF delta = position - (start + direction * projection);
        const double squaredDistance = QPointF::dotProduct(delta, delta);

        if (squaredDistance < minimum) {
            minimum = squaredDistance;
            nearest = i;
        }
    }

    return nearest;
}

QPointF BezierWidget::boundedPoint(const QPointF &position) const
{
    return QPointF(qBound(0.0, position.x(), double(qMax(0, width() - 1))),
                   qBound(0.0, position.y(), double(qMax(0, height() - 1))));
}

QPointF BezierWidget::lerp(const QPointF &a, const QPointF &b, double t) const
{
    return a * (1.0 - t) + b * t;
}

void BezierWidget::drawSourcePolyline(QPainter &painter) const
{
    if (!m_showHelpers)
        return;

    painter.setPen(QPen(QColor(185, 185, 185), 1.0, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    for (int i = 0; i + 1 < m_points.size(); ++i)
        painter.drawLine(m_points[i], m_points[i + 1]);
}

void BezierWidget::drawCasteljau(QPainter &painter,
                                 const Segment &segment,
                                 const QColor &polygonColor,
                                 const QString &prefix) const
{
    if (!m_showCasteljau)
        return;

    const double t = m_casteljauT;
    const QPointF first0 = lerp(segment.p0, segment.p1, t);
    const QPointF first1 = lerp(segment.p1, segment.p2, t);
    const QPointF first2 = lerp(segment.p2, segment.p3, t);
    const QPointF second0 = lerp(first0, first1, t);
    const QPointF second1 = lerp(first1, first2, t);
    const QPointF onCurve = lerp(second0, second1, t);

    QColor firstColor = polygonColor;
    firstColor.setAlpha(210);
    QColor secondColor = polygonColor;
    secondColor.setAlpha(165);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(firstColor, 1.55));
    painter.drawLine(first0, first1);
    painter.drawLine(first1, first2);

    painter.setPen(QPen(secondColor, 1.55));
    painter.drawLine(second0, second1);

    painter.setPen(Qt::NoPen);
    painter.setBrush(firstColor);
    for (const QPointF &p : {first0, first1, first2})
        painter.drawEllipse(p, 3.7, 3.7);
    painter.setBrush(secondColor);
    painter.drawEllipse(second0, 4.1, 4.1);
    painter.drawEllipse(second1, 4.1, 4.1);
    painter.setBrush(polygonColor);
    painter.drawEllipse(onCurve, 4.4, 4.4);

    if (!m_showLabels)
        return;

    QFont font = painter.font();
    font.setPointSize(qMax(8, font.pointSize() > 0 ? font.pointSize() - 1 : 9));
    font.setBold(false);
    painter.save();
    painter.setFont(font);
    painter.setPen(polygonColor);

    const QString superscript1 = QString::fromUtf8("¹");
    const QString superscript2 = QString::fromUtf8("²");
    const QString superscript3 = QString::fromUtf8("³");

    auto mark = [&](const QPointF &p, int sub, const QString &sup,
                    const QPointF &offset) {
        painter.drawText(QRectF(p + offset, QSizeF(50.0, 19.0)),
                         prefix + digitSubscript(sub) + sup);
    };

    mark(first0, 0, superscript1, QPointF(-23, -19));
    mark(first1, 1, superscript1, QPointF(-9, -21));
    mark(first2, 2, superscript1, QPointF(8, -19));
    mark(second0, 0, superscript2, QPointF(-22, 7));
    mark(second1, 1, superscript2, QPointF(7, 7));
    mark(onCurve, 0, superscript3, QPointF(-12, 9));
    painter.restore();
}

void BezierWidget::drawSegments(QPainter &painter,
                                const QVector<Segment> &segments) const
{
    if (segments.isEmpty())
        return;

    for (int i = 0; i < segments.size(); ++i) {
        const Segment &s = segments[i];
        const QColor polygonColor = polygonColorForIndex(i);
        const QColor curveColor = curveColorForIndex(i);
        const QString prefix(1, QChar('P' + (i % 11)));

        if (m_showHelpers) {
            painter.setPen(QPen(polygonColor, 1.7));
            painter.setBrush(Qt::NoBrush);
            painter.drawLine(s.p0, s.p1);
            painter.drawLine(s.p1, s.p2);
            painter.drawLine(s.p2, s.p3);
        }

        drawCasteljau(painter, s, polygonColor, prefix);

        QPainterPath path(s.p0);
        path.cubicTo(s.p1, s.p2, s.p3);
        painter.setPen(QPen(curveColor, 2.8));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    if (m_showHelpers && segments.size() > 1) {
        painter.setPen(QPen(QColor(35, 35, 35), 1.2));
        painter.setBrush(Qt::white);
        for (int i = 0; i + 1 < segments.size(); ++i)
            painter.drawEllipse(segments[i].p3, 4.8, 4.8);
    }
}

void BezierWidget::drawLabels(QPainter &painter,
                              const QVector<Segment> &segments) const
{
    if (!m_showLabels || segments.isEmpty())
        return;

    QFont font = painter.font();
    font.setBold(true);
    painter.save();
    painter.setFont(font);

    auto drawTextNear = [&](const QPointF &p, const QString &text,
                            const QColor &color, const QPointF &offset) {
        painter.setPen(color);
        painter.drawText(QRectF(p + offset, QSizeF(76.0, 21.0)), text);
    };

    for (int i = 0; i < segments.size(); ++i) {
        const Segment &s = segments[i];
        const QString prefix(1, QChar('P' + (i % 11)));
        const QColor color = polygonColorForIndex(i);

        if (i == 0)
            drawTextNear(s.p0, prefix + digitSubscript(0), color, QPointF(-21, 5));

        drawTextNear(s.p1, prefix + digitSubscript(1), color, QPointF(-10, -25));
        drawTextNear(s.p2, prefix + digitSubscript(2), color, QPointF(5, -26));

        if (i + 1 == segments.size()) {
            drawTextNear(s.p3, prefix + digitSubscript(3), color, QPointF(7, 5));
        } else {
            const QString nextPrefix(1, QChar('P' + ((i + 1) % 11)));
            drawTextNear(s.p3,
                         prefix + digitSubscript(3) + QStringLiteral("/")
                             + nextPrefix + digitSubscript(0),
                         QColor(50, 50, 50), QPointF(7, -23));
        }
    }
    painter.restore();
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    const QPointF position = event->pos();

    if (event->button() == Qt::RightButton) {
        const int index = pointAt(position);
        if (index >= 0) {
            m_points.remove(index);
            m_draggedIndex = -1;
            m_dragOffset = QPointF();
            update();
        }
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    const int index = pointAt(position);
    if (index >= 0) {
        m_draggedIndex = index;
        m_dragOffset = m_points[index] - position;
    } else if (event->modifiers() & Qt::ShiftModifier) {
        const int edge = nearestEdge(position);
        if (edge >= 0) {
            m_points.insert(edge + 1, boundedPoint(position));
            m_draggedIndex = edge + 1;
        } else {
            m_points.append(boundedPoint(position));
            m_draggedIndex = m_points.size() - 1;
        }
        m_dragOffset = QPointF();
    } else {
        m_points.append(boundedPoint(position));
        m_draggedIndex = m_points.size() - 1;
        m_dragOffset = QPointF();
    }

    update();
}

void BezierWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedIndex >= 0 && m_draggedIndex < m_points.size()
        && (event->buttons() & Qt::LeftButton)) {
        m_points[m_draggedIndex] = boundedPoint(QPointF(event->pos()) + m_dragOffset);
        update();
    }
}

void BezierWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_draggedIndex = -1;
        m_dragOffset = QPointF();
        update();
    }
}

void BezierWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        clear();
        return;
    }

    if (event->key() == Qt::Key_H) {
        m_showHelpers = !m_showHelpers;
        update();
        return;
    }

    if (event->key() == Qt::Key_N) {
        m_showLabels = !m_showLabels;
        update();
        return;
    }

    if (event->key() == Qt::Key_C) {
        m_showCasteljau = !m_showCasteljau;
        update();
        return;
    }

    QWidget::keyPressEvent(event);
}

void BezierWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QVector<Segment> segments = buildSegments();

    painter.setPen(QColor(75, 75, 75));
    painter.drawText(QRectF(12.0, 9.0, qMax(0, width() - 24), 24.0),
                     QString::fromUtf8("Shift+ЛКМ — вставить; ПКМ — удалить; H — линии; N — подписи; C — де Кастельжо (t=0,5)"));

    drawSourcePolyline(painter);
    drawSegments(painter, segments);
    drawLabels(painter, segments);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(205, 45, 45));
    for (const QPointF &point : m_points)
        painter.drawEllipse(point, 6.5, 6.5);

    if (m_draggedIndex >= 0 && m_draggedIndex < m_points.size()) {
        painter.setPen(QPen(QColor(230, 170, 35), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(m_points[m_draggedIndex], 10.5, 10.5);
    }
}
