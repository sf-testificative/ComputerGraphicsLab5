#include "bezierwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QtMath>

BezierWidget::BezierWidget(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_points << QPointF(100, 400) << QPointF(300, 100) << QPointF(500, 400) << QPointF(700, 200);
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m_points.size(); ++i) {
            if (QLineF(event->pos(), m_points[i]).length() < 12) {
                m_draggedIndex = i;
                m_dragOffset = m_points[i] - event->pos();
                return;
            }
        }
        m_points.append(event->pos());
        update();
    } else if (event->button() == Qt::RightButton) {
        for (int i = 0; i < m_points.size(); ++i) {
            if (QLineF(event->pos(), m_points[i]).length() < 12) {
                m_points.remove(i);
                update();
                return;
            }
        }
    }
}

void BezierWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedIndex != -1) {
        m_points[m_draggedIndex] = event->pos() + m_dragOffset;
        update();
    }
}

void BezierWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggedIndex = -1;
}

QPointF BezierWidget::getControlPoint1(int i) const
{
    if (i == 0) return m_points[0];
    QPointF p0 = m_points[i - 1];
    QPointF p1 = m_points[i];
    QPointF p2 = m_points[i + 1];
    return p1 + (p2 - p0) / 6.0;
}

QPointF BezierWidget::getControlPoint2(int i) const
{
    if (i == m_points.size() - 1) return m_points[i];
    QPointF p0 = m_points[i - 1];
    QPointF p1 = m_points[i];
    QPointF p2 = m_points[i + 1];
    return p1 - (p2 - p0) / 6.0;
}

void BezierWidget::drawBezier(QPainter &painter)
{
    if (m_points.size() < 2) return;

    QPainterPath path;
    path.moveTo(m_points[0]);

    for (int i = 0; i < m_points.size() - 1; ++i) {
        QPointF c1 = getControlPoint1(i);
        QPointF c2 = getControlPoint2(i + 1);
        path.cubicTo(c1, c2, m_points[i + 1]);
    }

    painter.setPen(QPen(Qt::blue, 3));
    painter.drawPath(path);

    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    for (int i = 0; i < m_points.size() - 1; ++i) {
        QPointF c1 = getControlPoint1(i);
        QPointF c2 = getControlPoint2(i + 1);
        painter.drawLine(m_points[i], c1);
        painter.drawLine(m_points[i + 1], c2);
    }
}

void BezierWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::red);
    for (const QPointF &p : m_points) {
        painter.drawEllipse(p, 6, 6);
    }

    drawBezier(painter);
}