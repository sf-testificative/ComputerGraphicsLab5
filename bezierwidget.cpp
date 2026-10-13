#include "bezierwidget.h"

#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtGlobal>
#include <cmath>
#include <limits>

BezierWidget::BezierWidget(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
}

void BezierWidget::clear()
{
    m_points.clear();
    m_draggedIndex = -1;
    update();
}

QPointF BezierWidget::bezierPoint(const QPointF &p0, const QPointF &p1,
                                  const QPointF &p2, const QPointF &p3, double t) const
{
    const double u = 1.0 - t;
    const double b0 = u * u * u;
    const double b1 = 3.0 * u * u * t;
    const double b2 = 3.0 * u * t * t;
    const double b3 = t * t * t;
    return QPointF(b0 * p0.x() + b1 * p1.x() + b2 * p2.x() + b3 * p3.x(),
                   b0 * p0.y() + b1 * p1.y() + b2 * p2.y() + b3 * p3.y());
}

QPointF BezierWidget::controlPoint1(int i) const
{
    if (i == 0)
        return m_points[0] + (m_points[1] - m_points[0]) / 3.0;
    return m_points[i] + (m_points[i + 1] - m_points[i - 1]) / 6.0;
}

QPointF BezierWidget::controlPoint2(int i) const
{
    if (i + 1 == m_points.size() - 1)
        return m_points[i + 1] - (m_points[i + 1] - m_points[i]) / 3.0;
    return m_points[i + 1] - (m_points[i + 2] - m_points[i]) / 6.0;
}

int BezierWidget::pointAt(const QPointF &position) const
{
    for (int i = m_points.size() - 1; i >= 0; --i) {
        if (QLineF(position, m_points[i]).length() <= 12.0)
            return i;
    }
    return -1;
}

int BezierWidget::nearestSegment(const QPointF &position) const
{
    if (m_points.size() < 2)
        return -1;

    int selected = -1;
    double bestDistanceSquared = std::numeric_limits<double>::max();
    for (int i = 0; i + 1 < m_points.size(); ++i) {
        const QPointF p0 = m_points[i];
        const QPointF p1 = controlPoint1(i);
        const QPointF p2 = controlPoint2(i);
        const QPointF p3 = m_points[i + 1];
        const int steps = qMax(32, qMin(500, int(std::ceil(QLineF(p0, p1).length()
                                                           + QLineF(p1, p2).length() + QLineF(p2, p3).length()))));
        QPointF previous = p0;
        for (int s = 1; s <= steps; ++s) {
            const QPointF current = bezierPoint(p0, p1, p2, p3, double(s) / steps);
            const QPointF vector = current - previous;
            const QPointF delta = position - previous;
            const double lengthSquared = vector.x() * vector.x() + vector.y() * vector.y();
            const double t = lengthSquared > 0.0
                                 ? qBound(0.0, (delta.x() * vector.x() + delta.y() * vector.y()) / lengthSquared, 1.0)
                                 : 0.0;
            const QPointF nearest = previous + vector * t;
            const QPointF difference = position - nearest;
            const double distanceSquared = difference.x() * difference.x() + difference.y() * difference.y();
            if (distanceSquared < bestDistanceSquared) {
                bestDistanceSquared = distanceSquared;
                selected = i;
            }
            previous = current;
        }
    }
    return selected;
}

QPointF BezierWidget::boundedPoint(const QPointF &position) const
{
    return QPointF(qBound(0.0, position.x(), double(qMax(0, width() - 1))),
                   qBound(0.0, position.y(), double(qMax(0, height() - 1))));
}

void BezierWidget::drawPolyline(QPainter &painter)
{
    if (m_points.size() < 2)
        return;
    painter.setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));
    for (int i = 0; i + 1 < m_points.size(); ++i)
        painter.drawLine(m_points[i], m_points[i + 1]);
}

void BezierWidget::drawBezierCurve(QPainter &painter)
{
    if (m_points.size() < 2)
        return;

    painter.setPen(QPen(Qt::blue, 2));
    QPainterPath path(m_points.first());
    for (int i = 0; i + 1 < m_points.size(); ++i)
        path.cubicTo(controlPoint1(i), controlPoint2(i), m_points[i + 1]);
    painter.drawPath(path);
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    const QPointF position = event->pos();

    if (event->button() == Qt::RightButton) {
        const int index = pointAt(position);
        if (index >= 0) {
            m_draggedIndex = -1;
            m_points.remove(index);
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
        const int segment = nearestSegment(position);
        if (segment >= 0) {
            m_points.insert(segment + 1, boundedPoint(position));
            m_draggedIndex = segment + 1;
            m_dragOffset = QPointF();
        } else {
            m_points.append(boundedPoint(position));
            m_draggedIndex = m_points.size() - 1;
            m_dragOffset = QPointF();
        }
    } else {
        m_points.append(boundedPoint(position));
        m_draggedIndex = -1;
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
        update();
    }
}

void BezierWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        clear();
        return;
    }
    QWidget::keyPressEvent(event);
}

void BezierWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawPolyline(painter);
    if (m_points.size() >= 2) {
        painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
        for (int i = 0; i + 1 < m_points.size(); ++i) {
            painter.drawLine(m_points[i], controlPoint1(i));
            painter.drawLine(m_points[i + 1], controlPoint2(i));
        }
    }
    drawBezierCurve(painter);

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::red);
    for (const QPointF &point : m_points)
        painter.drawEllipse(point, 6, 6);

    if (m_draggedIndex >= 0 && m_draggedIndex < m_points.size()) {
        painter.setPen(QPen(Qt::darkYellow, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(m_points[m_draggedIndex], 10, 10);
    }
}
