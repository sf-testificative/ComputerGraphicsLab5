#ifndef MIDPOINTWIDGET_H
#define MIDPOINTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QRandomGenerator>

class MidpointWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MidpointWidget(QWidget *parent = nullptr);
    void setSeed(quint32 seed, bool useSeed);

    void startNew(double roughness, int totalSteps);
    void nextStep();
    void finishAll();
    void reset();

    bool isFinished() const { return m_currentStep >= m_totalSteps; }
    bool isEmpty() const { return m_heights.size() < 2; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_heights;
    double m_roughness = 0.5;
    int m_totalSteps = 6;
    int m_currentStep = 0;

    quint32 m_seed = 0;
    bool m_useSeed = false;
    QRandomGenerator m_rng;

    double random01();
    void performStep();
};

#endif // MIDPOINTWIDGET_H