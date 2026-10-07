#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class LSystemWidget;
class MidpointWidget;
class BezierWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadLSystemFile();
    void updateLSystem();
    void generateMidpoint();
    void resetMidpoint();

private:
    QTabWidget *m_tabs;

    // L-системы
    LSystemWidget *m_lsystemWidget;
    QSpinBox *m_lsystemIterations;
    QCheckBox *m_lsystemRandom;
    QPushButton *m_lsystemLoadBtn;

    // Midpoint
    MidpointWidget *m_midpointWidget;
    QSlider *m_roughnessSlider;
    QSpinBox *m_stepsSpinBox;
    QPushButton *m_generateMidpointBtn;
    QPushButton *m_resetMidpointBtn;

    // Bezier
    BezierWidget *m_bezierWidget;

    void setupLSystemTab();
    void setupMidpointTab();
    void setupBezierTab();
};

#endif // MAINWINDOW_H