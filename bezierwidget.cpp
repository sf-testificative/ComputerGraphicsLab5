#include "bezierwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>

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

QPointF BezierWidget::bezierPoint(const QPointF &P0, const QPointF &P1, const QPointF &P2, const QPointF &P3, double t) const
{
    double u  = 1.0 - t;
    double u2 = u * u;
    double u3 = u2 * u;
    double t2 = t * t;
    double t3 = t2 * t;

    double b0 = u3;
    double b1 = 3.0 * u2 * t;
    double b2 = 3.0 * u  * t2;
    double b3 = t3;

    double x = b0 * P0.x() + b1 * P1.x() + b2 * P2.x() + b3 * P3.x();
    double y = b0 * P0.y() + b1 * P1.y() + b2 * P2.y() + b3 * P3.y();

    return QPointF(x, y);
}

QPointF BezierWidget::controlPoint1(int i) const
{
    if (i == 0) {
        return m_points[0] + (m_points[1] - m_points[0]) / 3.0;
    }
    QPointF pPrev = m_points[i - 1];
    QPointF pCurr = m_points[i];
    QPointF pNext = m_points[i + 1];
    return pCurr + (pNext - pPrev) / 6.0;
}

QPointF BezierWidget::controlPoint2(int i) const
{
    if (i + 1 >= m_points.size() - 1) {
        return m_points[i + 1] - (m_points[i + 1] - m_points[i]) / 3.0;
    }
    QPointF pCurr = m_points[i];
    QPointF pNext = m_points[i + 1];
    QPointF pNextNext = m_points[i + 2];
    return pNext - (pNextNext - pCurr) / 6.0;
}

void BezierWidget::drawPolyline(QPainter &painter)
{
    if (m_points.size() < 2) return;

    painter.setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));
    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter.drawLine(m_points[i], m_points[i + 1]);
    }
}

void BezierWidget::drawBezierCurve(QPainter &painter)
{
    if (m_points.size() < 2) return;

    const int STEPS = 50;
    painter.setPen(QPen(Qt::blue, 2));

    for (int i = 0; i < m_points.size() - 1; ++i) {
        QPointF P0 = m_points[i];
        QPointF P3 = m_points[i + 1];
        QPointF P1 = controlPoint1(i);
        QPointF P2 = controlPoint2(i);

        QPointF prev = bezierPoint(P0, P1, P2, P3, 0.0);
        for (int s = 1; s <= STEPS; ++s) {
            double t = double(s) / STEPS;
            QPointF curr = bezierPoint(P0, P1, P2, P3, t);
            painter.drawLine(prev, curr);
            prev = curr;
        }
    }
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();

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
        for (int i = 0; i < m_points.size() - 1; ++i) {
            painter.drawLine(m_points[i],     controlPoint1(i));
            painter.drawLine(m_points[i + 1], controlPoint2(i));
        }
    }

    drawBezierCurve(painter);
    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::red);
    for (int i = 0; i < m_points.size(); ++i) {
        painter.drawEllipse(m_points[i], 6, 6);
    }

    if (m_draggedIndex != -1) {
        painter.setPen(QPen(Qt::darkYellow, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(m_points[m_draggedIndex], 10, 10);
    }
}