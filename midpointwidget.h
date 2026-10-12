#ifndef MIDPOINTWIDGET_H
#define MIDPOINTWIDGET_H

#include <QWidget>
#include <QVector>

class MidpointWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MidpointWidget(QWidget *parent = nullptr);

    void startNew(double roughness, int totalSteps);

    void nextStep();

    void finishAll();

    void reset();

    bool isFinished() const { return m_currentStep >= m_totalSteps; }
    bool isEmpty()    const { return m_heights.size() < 2; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_heights;
    double m_roughness = 0.5;
    int m_totalSteps   = 6;
    int m_currentStep  = 0;

    void performStep();
};

#endif // MIDPOINTWIDGET_H