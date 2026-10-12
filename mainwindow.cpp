#include "mainwindow.h"
#include "lsystemwidget.h"
#include "midpointwidget.h"
#include "bezierwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle("Lab 5");
    resize(1000, 800);

    m_tabs = new QTabWidget(this);
    setCentralWidget(m_tabs);

    setupLSystemTab();
    setupMidpointTab();
    setupBezierTab();
}

MainWindow::~MainWindow() {}

void MainWindow::setupLSystemTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(tab);
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    m_lsystemWidget = new LSystemWidget();

    m_lsystemLoadBtn = new QPushButton("Загрузить L-систему");
    m_lsystemIterations = new QSpinBox();
    m_lsystemIterations->setRange(0, 10);
    m_lsystemIterations->setValue(4);
    m_lsystemIterations->setPrefix("Итераций: ");

    m_lsystemRandom = new QCheckBox("Случайность угла");

    controlsLayout->addWidget(m_lsystemLoadBtn);
    controlsLayout->addWidget(m_lsystemIterations);
    controlsLayout->addWidget(m_lsystemRandom);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(m_lsystemWidget);

    m_tabs->addTab(tab, "1. L-системы");

    connect(m_lsystemLoadBtn, &QPushButton::clicked, this, &MainWindow::loadLSystemFile);
    connect(m_lsystemIterations, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateLSystem);
    connect(m_lsystemRandom, &QCheckBox::toggled, this, &MainWindow::updateLSystem);
}

void MainWindow::loadLSystemFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "Открыть L-систему", "", "Text Files (*.txt)");
    if (filename.isEmpty()) return;

    m_lsystemWidget->loadFromFile(filename);
    m_lsystemWidget->setIterations(m_lsystemIterations->value());
    m_lsystemWidget->setRandomness(m_lsystemRandom->isChecked());
}

void MainWindow::updateLSystem()
{
    m_lsystemWidget->setIterations(m_lsystemIterations->value());
    m_lsystemWidget->setRandomness(m_lsystemRandom->isChecked());
    m_lsystemWidget->update();
}

void MainWindow::setupMidpointTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(tab);
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    m_midpointWidget = new MidpointWidget();

    m_roughnessSlider = new QSlider(Qt::Horizontal);
    m_roughnessSlider->setRange(1, 100);
    m_roughnessSlider->setValue(50);

    QLabel *roughLabel = new QLabel("Шероховатость:");
    QLabel *roughValLabel = new QLabel("0.50");
    connect(m_roughnessSlider, &QSlider::valueChanged, [roughValLabel](int val) {
        roughValLabel->setText(QString::number(val / 100.0, 'f', 2));
    });

    m_stepsSpinBox = new QSpinBox();
    m_stepsSpinBox->setRange(1, 12);
    m_stepsSpinBox->setValue(6);
    m_stepsSpinBox->setPrefix("Шагов: ");

    m_generateMidpointBtn = new QPushButton("Сгенерировать");
    m_resetMidpointBtn = new QPushButton("Сбросить");

    controlsLayout->addWidget(roughLabel);
    controlsLayout->addWidget(m_roughnessSlider);
    controlsLayout->addWidget(roughValLabel);
    controlsLayout->addWidget(m_stepsSpinBox);
    controlsLayout->addWidget(m_generateMidpointBtn);
    controlsLayout->addWidget(m_resetMidpointBtn);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(m_midpointWidget);

    m_tabs->addTab(tab, "2. Midpoint Displacement");

    connect(m_generateMidpointBtn, &QPushButton::clicked, this, &MainWindow::generateMidpoint);
    connect(m_resetMidpointBtn, &QPushButton::clicked, this, &MainWindow::resetMidpoint);
}

void MainWindow::generateMidpoint()
{
    double roughness = m_roughnessSlider->value() / 100.0;
    int steps = m_stepsSpinBox->value();
    m_midpointWidget->generate(steps, roughness);
}

void MainWindow::resetMidpoint()
{
    m_midpointWidget->reset();
}

void MainWindow::setupBezierTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(tab);

    m_bezierWidget = new BezierWidget();
    m_bezierWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(m_bezierWidget, 1);

    m_tabs->addTab(tab, "3. Кривые Безье");
}