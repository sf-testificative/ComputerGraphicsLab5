#ifndef BEZIERWIDGET_H
#define BEZIERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class QPainter;
class QMouseEvent;
class QKeyEvent;
class QPaintEvent;

class BezierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BezierWidget(QWidget *parent = nullptr);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<QPointF> m_points;
    int m_draggedIndex = -1;
    QPointF m_dragOffset;

    QPointF bezierPoint(const QPointF &p0, const QPointF &p1,
                        const QPointF &p2, const QPointF &p3, double t) const;
    QPointF controlPoint1(int segmentIndex) const;
    QPointF controlPoint2(int segmentIndex) const;
    int pointAt(const QPointF &position) const;
    int nearestSegment(const QPointF &position) const;
    QPointF boundedPoint(const QPointF &position) const;
    void drawPolyline(QPainter &painter);
    void drawBezierCurve(QPainter &painter);
};

#endif
