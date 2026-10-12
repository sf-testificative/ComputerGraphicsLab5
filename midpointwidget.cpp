#include "midpointwidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QPainterPath>

MidpointWidget::MidpointWidget(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    reset();
}

void MidpointWidget::reset()
{
    m_heights.clear();
    m_heights.append(QRandomGenerator::global()->generateDouble());
    m_heights.append(QRandomGenerator::global()->generateDouble());
    m_currentStep = 0;
    update();
}

void MidpointWidget::startNew(double roughness, int totalSteps)
{
    m_roughness  = qBound(0.0, roughness, 1.0);
    m_totalSteps = qBound(1, totalSteps, 12);
    reset();
}

void MidpointWidget::performStep()
{
    QVector<double> newHeights;
    int size = m_heights.size();
    newHeights.reserve(size * 2 - 1);
    double L = 1.0 / (size - 1);

    for (int i = 0; i < size - 1; ++i) {
        newHeights.append(m_heights[i]);

        double hL = m_heights[i];
        double hR = m_heights[i + 1];

        double r = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        double displacement = r * m_roughness * L;

        double mid = (hL + hR) / 2.0 + displacement;
        mid = qBound(0.0, mid, 1.0);
        newHeights.append(mid);
    }
    newHeights.append(m_heights.last());

    m_heights = newHeights;
    m_currentStep++;
}

void MidpointWidget::nextStep()
{
    if (isFinished()) return;
    performStep();
    update();
}

void MidpointWidget::finishAll()
{
    while (!isFinished()) {
        performStep();
    }
    update();
}

void MidpointWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_heights.size() < 2) return;

    double xStep  = double(width()) / (m_heights.size() - 1);
    double yScale = height() * 0.8;
    double yOffset = height() * 0.1;

    QPainterPath path;
    path.moveTo(0, height() - (m_heights[0] * yScale + yOffset));

    for (int i = 1; i < m_heights.size(); ++i) {
        double x = i * xStep;
        double y = height() - (m_heights[i] * yScale + yOffset);
        path.lineTo(x, y);
    }

    QPainterPath fillPath = path;
    fillPath.lineTo(width(), height());
    fillPath.lineTo(0, height());
    fillPath.closeSubpath();

    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(100, 150, 200));
    gradient.setColorAt(1, QColor(200, 220, 240));
    painter.fillPath(fillPath, gradient);

    painter.setPen(QPen(Qt::darkBlue, 2));
    painter.drawPath(path);

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(11);
    painter.setFont(font);
    painter.drawText(10, 20,
                     QString("Шаг %1 из %2   (точек: %3)")
                         .arg(m_currentStep)
                         .arg(m_totalSteps)
                         .arg(m_heights.size()));
}