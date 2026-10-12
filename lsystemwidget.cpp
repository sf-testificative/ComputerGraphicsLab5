#include "lsystemwidget.h"
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

LSystemWidget::LSystemWidget(QWidget *parent) : QWidget(parent) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void LSystemWidget::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    QStringList head = in.readLine().split(' ', Qt::SkipEmptyParts);
    if (head.size() >= 3) {
        m_axiom = head[0];
        m_angle = head[1].toDouble();
        m_startAngle = head[2].toDouble();
    }

    m_rules.clear();
    while (!in.atEnd()) {
        QStringList p = in.readLine().split("->");
        if (p.size() == 2 && !p[0].trimmed().isEmpty())
            m_rules[p[0].trimmed()[0]] = p[1].trimmed();
    }

    m_loaded = true;
    generate();
    update();
}

void LSystemWidget::setIterations(int n) {
    m_iterations = n;
    if (m_loaded) generate();
    update();
}

void LSystemWidget::setRandomness(bool enabled) {
    m_random = enabled;
    if (m_loaded) generate();
    update();
}

void LSystemWidget::generate()
{
    QString current = m_axiom;
    for (int i = 0; i < m_iterations; i++) {
        QString next;
        for (QChar c : current)
            next += m_rules.contains(c) ? m_rules[c] : QString(c);
        current = next;
    }
    interpret(current);
}
#include <iostream>
void LSystemWidget::interpret(const QString &s)
{

    std::cout << s.toStdString() << std::endl;
    m_lines.clear();
    QStack<QPair<QPointF, double>> stack;
    QPointF pos(0, 0);
    double ang = m_startAngle;
    double step = 10.0;
    int depth = 0;

    for (QChar c : s) {
        if (c == 'F' || c == 'G') {
            double r = qDegreesToRadians(ang);
            QPointF np = pos + QPointF(step * qCos(r), step * qSin(r));
            m_lines.append({ QLineF(pos, np), depth });
            pos = np;
        }
        else if (c == '+') {
            double d = m_angle;
            if (m_random) d += QRandomGenerator::global()->generateDouble() * 30.0 - 15.0;
            ang += d;
        }
        else if (c == '-') {
            double d = m_angle;
            if (m_random) d += QRandomGenerator::global()->generateDouble() * 30.0 - 15.0;
            ang -= d;
        }
        else if (c == '[') { stack.push({pos, ang}); depth++; }
        else if (c == ']') {
            if (!stack.isEmpty()) { pos = stack.top().first; ang = stack.top().second; stack.pop(); depth--; }
        }
    }
    std::cout << "Lines: " << m_lines.size() << std::endl;
    if (!m_lines.isEmpty()) {
        std::cout << "First: " << m_lines.first().line.x1() << ","
                  << m_lines.first().line.y1() << " -> "
                  << m_lines.first().line.x2() << ","
                  << m_lines.first().line.y2() << std::endl;
    }
}

void LSystemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (!m_loaded) {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "Загрузите файл с L-системой");
        return;
    }
    if (m_lines.isEmpty()) return;

    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (auto &l : m_lines) {
        minX = qMin(minX, qMin(l.line.x1(), l.line.x2()));
        maxX = qMax(maxX, qMax(l.line.x1(), l.line.x2()));
        minY = qMin(minY, qMin(l.line.y1(), l.line.y2()));
        maxY = qMax(maxY, qMax(l.line.y1(), l.line.y2()));
    }

    double w = maxX - minX;
    double h = maxY - minY;
    if (w < 1e-6) w = 1.0;
    if (h < 1e-6) h = 1.0;

    double margin = 20.0;
    double scale = qMin((width()  - 2*margin) / w,
                        (height() - 2*margin) / h);
    if (scale <= 0) scale = 1.0;

    double offX = (width()  - w * scale) / 2.0;
    double offY = (height() - h * scale) / 2.0;

    for (auto &l : m_lines) {
        QPointF p1(offX + (l.line.x1() - minX) * scale,
                   offY + (maxY - l.line.y1()) * scale);
        QPointF p2(offX + (l.line.x2() - minX) * scale,
                   offY + (maxY - l.line.y2()) * scale);

        double lw = 2.0;
        QColor col = Qt::black;
        if (l.depth > 0) {
            lw = qMax(0.5, 4.0 * qPow(0.7, l.depth));
            col = QColor(qBound(0, 139 - l.depth*10, 255),
                         qBound(0,  69 + l.depth*20, 255),
                         qBound(0,  19 + l.depth*5,  255));
        }
        p.setPen(QPen(col, lw));
        p.drawLine(p1, p2);

        p.setPen(Qt::NoPen);
        p.setBrush(Qt::red);
        const double dotRadius = 3.0;
        p.drawEllipse(p1, dotRadius, dotRadius);
        p.drawEllipse(p2, dotRadius, dotRadius);
    }
}