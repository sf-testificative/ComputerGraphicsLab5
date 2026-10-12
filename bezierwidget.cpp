#include "bezierwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QtMath>

BezierWidget::BezierWidget(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(false);
}

QPointF BezierWidget::controlPoint1(int i) const
{
    if (i == 0) {
        return m_points[0];
    }
    QPointF pPrev = m_points[i - 1];
    QPointF pCurr = m_points[i];
    QPointF pNext = m_points[i + 1];
    return pCurr + (pNext - pPrev) / 6.0;
}

QPointF BezierWidget::controlPoint2(int i) const
{
    if (i >= m_points.size() - 2) {
        return m_points.last();
    }
    QPointF pPrev = m_points[i];
    QPointF pCurr = m_points[i + 1];
    QPointF pNext = m_points[i + 2];
    return pCurr - (pNext - pPrev) / 6.0;
}

void BezierWidget::drawBezier(QPainter &painter)
{
    if (m_points.size() < 2) return;

    QPainterPath path;
    path.moveTo(m_points[0]);

    for (int i = 0; i < m_points.size() - 1; ++i) {
        QPointF c1 = controlPoint1(i);
        QPointF c2 = controlPoint2(i + 1);
        path.cubicTo(c1, c2, m_points[i + 1]);
    }

    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    painter.setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));
    for (int i = 0; i < m_points.size() - 1; ++i) {
        QPointF c1 = controlPoint1(i);
        QPointF c2 = controlPoint2(i + 1);
        painter.drawLine(m_points[i], c1);
        painter.drawLine(m_points[i + 1], c2);
    }
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m_points.size(); ++i) {
            if (QLineF(event->pos(), m_points[i]).length() < 12) {
                m_draggedIndex = i;
                m_dragOffset = m_points[i] - event->pos();
                update();
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

void BezierWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawBezier(painter);

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::red);
    for (int i = 0; i < m_points.size(); ++i) {
        painter.drawEllipse(m_points[i], 6, 6);
    }

    if (m_draggedIndex != -1) {
        painter.setPen(QPen(Qt::yellow, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(m_points[m_draggedIndex], 10, 10);
    }
}