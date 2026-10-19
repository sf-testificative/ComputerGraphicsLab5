#ifndef BEZIERWIDGET_H
#define BEZIERWIDGET_H

#include <QPointF>
#include <QString>
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;

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
    struct Segment {
        QPointF p0;
        QPointF p1;
        QPointF p2;
        QPointF p3;
    };

    QVector<QPointF> m_points;
    int m_draggedIndex = -1;
    QPointF m_dragOffset;
    bool m_showHelpers = true;
    bool m_showLabels = true;
    bool m_showCasteljau = true;
    double m_casteljauT = 0.5;

    QVector<Segment> buildSegments() const;
    int pointAt(const QPointF &position) const;
    int nearestEdge(const QPointF &position) const;
    QPointF boundedPoint(const QPointF &position) const;
    QPointF lerp(const QPointF &a, const QPointF &b, double t) const;
    void drawSourcePolyline(QPainter &painter) const;
    void drawSegments(QPainter &painter, const QVector<Segment> &segments) const;
    void drawCasteljau(QPainter &painter, const Segment &segment,
                       const QColor &polygonColor, const QString &prefix) const;
    void drawLabels(QPainter &painter, const QVector<Segment> &segments) const;
};

#endif
