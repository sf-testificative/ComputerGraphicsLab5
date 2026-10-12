#include "mainwindow.h"
#include "lsystemwidget.h"
#include "midpointwidget.h"
#include "bezierwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QIntValidator>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
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
    m_lsystemWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_lsystemLoadBtn = new QPushButton("Загрузить L-систему");
    m_lsystemIterations = new QSpinBox();
    m_lsystemIterations->setRange(0, 10);
    m_lsystemIterations->setValue(0);
    m_lsystemIterations->setPrefix("Итераций: ");

    m_lsystemRandom = new QCheckBox("Случайность углов");

    controlsLayout->addWidget(m_lsystemLoadBtn);
    controlsLayout->addWidget(m_lsystemIterations);
    controlsLayout->addWidget(m_lsystemRandom);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(m_lsystemWidget, 1);

    m_tabs->addTab(tab, "1. L-системы");

    connect(m_lsystemLoadBtn, &QPushButton::clicked, this, &MainWindow::loadLSystemFile);
    connect(m_lsystemIterations, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateLSystem);
    connect(m_lsystemRandom, &QCheckBox::toggled, this, &MainWindow::updateLSystem);
}

void MainWindow::loadLSystemFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this, "Открыть L-систему", "", "Text Files (*.txt)");
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
    m_midpointWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLabel *roughLabel = new QLabel("R =");
    m_roughnessSlider = new QSlider(Qt::Horizontal);
    m_roughnessSlider->setRange(1, 100);
    m_roughnessSlider->setValue(50);
    m_roughnessSlider->setFixedWidth(150);

    m_roughnessLabel = new QLabel("0.50");
    connect(m_roughnessSlider, &QSlider::valueChanged, this, [this](int val) {
        m_roughnessLabel->setText(QString::number(val / 100.0, 'f', 2));
    });

    m_stepsSpinBox = new QSpinBox();
    m_stepsSpinBox->setRange(1, 12);
    m_stepsSpinBox->setValue(6);
    m_stepsSpinBox->setPrefix("Всего шагов: ");

    m_seedCheckBox = new QCheckBox("Фиксированный seed:");
    m_seedLineEdit = new QLineEdit();
    m_seedLineEdit->setValidator(new QIntValidator(0, 2147483647, this));
    m_seedLineEdit->setFixedWidth(100);
    m_seedLineEdit->setEnabled(false);

    connect(m_seedCheckBox, &QCheckBox::toggled, this, [this](bool on) {
        m_seedLineEdit->setEnabled(on);
        applySeedSetting();
    });
    connect(m_seedLineEdit, &QLineEdit::editingFinished, this, &MainWindow::applySeedSetting);

    m_startMidpointBtn = new QPushButton("Начать заново");
    m_nextStepBtn = new QPushButton("Следующий шаг");
    m_finishBtn = new QPushButton("Пропустить до конца");

    controlsLayout->addWidget(roughLabel);
    controlsLayout->addWidget(m_roughnessSlider);
    controlsLayout->addWidget(m_roughnessLabel);
    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(m_stepsSpinBox);
    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(m_seedCheckBox);
    controlsLayout->addWidget(m_seedLineEdit);
    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(m_startMidpointBtn);
    controlsLayout->addWidget(m_nextStepBtn);
    controlsLayout->addWidget(m_finishBtn);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(m_midpointWidget, 1);

    m_tabs->addTab(tab, "2. Midpoint Displacement");

    connect(m_startMidpointBtn, &QPushButton::clicked, this, &MainWindow::startMidpoint);
    connect(m_nextStepBtn, &QPushButton::clicked, this, &MainWindow::nextMidpointStep);
    connect(m_finishBtn, &QPushButton::clicked, this, &MainWindow::finishMidpoint);

    updateMidpointButtons();
}

void MainWindow::applySeedSetting()
{
    bool useSeed = m_seedCheckBox->isChecked();
    QString text = m_seedLineEdit->text().trimmed();

    if (useSeed && !text.isEmpty()) {
        quint32 seed = text.toUInt();
        m_midpointWidget->setSeed(seed, true);
    } else {
        m_midpointWidget->setSeed(0, false);
    }
}

void MainWindow::startMidpoint()
{
    applySeedSetting();
    double R = m_roughnessSlider->value() / 100.0;
    int steps = m_stepsSpinBox->value();
    m_midpointWidget->startNew(R, steps);
    updateMidpointButtons();
}

void MainWindow::nextMidpointStep()
{
    m_midpointWidget->nextStep();
    updateMidpointButtons();
}

void MainWindow::finishMidpoint()
{
    m_midpointWidget->finishAll();
    updateMidpointButtons();
}

void MainWindow::updateMidpointButtons()
{
    bool finished = m_midpointWidget->isFinished();
    m_nextStepBtn->setEnabled(!finished);
    m_finishBtn->setEnabled(!finished);
}

void MainWindow::setupBezierTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(tab);
    QHBoxLayout *topLayout = new QHBoxLayout();

    m_bezierClearBtn = new QPushButton("Очистить");
    m_bezierClearBtn->setFixedWidth(120);
    topLayout->addWidget(m_bezierClearBtn);
    m_bezierWidget = new BezierWidget();
    m_bezierWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_bezierWidget, 1);

    m_tabs->addTab(tab, "3. Кривые Безье");
    connect(m_bezierClearBtn, &QPushButton::clicked, this, &MainWindow::clearBezier);
}

void MainWindow::clearBezier()
{
    m_bezierWidget->clear();
}