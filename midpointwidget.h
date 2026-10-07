#ifndef MIDPOINTWIDGET_H
#define MIDPOINTWIDGET_H

#include <QWidget>
#include <QVector>

class MidpointWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MidpointWidget(QWidget *parent = nullptr);
    void generate(int steps, double roughness);
    void reset();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_heights;
    int m_currentStep;

    void midpointDisplacementStep();
};

#endif // MIDPOINTWIDGET_H