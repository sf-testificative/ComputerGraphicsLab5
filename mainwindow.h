#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
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

    void startMidpoint();
    void nextMidpointStep();
    void finishMidpoint();
    void applySeedSetting();

    void clearBezier();

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
    QLabel *m_roughnessLabel;
    QSpinBox *m_stepsSpinBox;
    QPushButton *m_startMidpointBtn;
    QPushButton *m_nextStepBtn;
    QPushButton *m_finishBtn;
    QCheckBox *m_seedCheckBox;
    QLineEdit *m_seedLineEdit;

    // Bezier
    BezierWidget *m_bezierWidget;
    QPushButton *m_bezierClearBtn;

    void setupLSystemTab();
    void setupMidpointTab();
    void setupBezierTab();
    void updateMidpointButtons();
};

#endif // MAINWINDOW_H