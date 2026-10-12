#ifndef BEZIERWIDGET_H
#define BEZIERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class BezierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BezierWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QVector<QPointF> m_points;
    int m_draggedIndex = -1;
    QPointF m_dragOffset;

    QPointF controlPoint1(int segmentIndex) const;
    QPointF controlPoint2(int segmentIndex) const;
    void drawBezier(QPainter &painter);
};

#endif // BEZIERWIDGET_H