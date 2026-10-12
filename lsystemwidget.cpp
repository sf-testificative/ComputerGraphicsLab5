#include "lsystemwidget.h"
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

LSystemWidget::LSystemWidget(QWidget *parent): QWidget(parent), m_iterations(4), m_random(false), m_loaded(false)
{
    setBackgroundRole(QPalette::BrightText);
    setAutoFillBackground(true);
    m_axiom = "";
    m_angle = 0.0;
    m_startAngle = 0.0;
}

void LSystemWidget::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filename;
        return;
    }

    QTextStream in(&file);
    QString header = in.readLine();
    QStringList headerParts = header.split(' ', Qt::SkipEmptyParts);
    if (headerParts.size() < 3) {
        qWarning() << "Неверный формат заголовка";
        return;
    }
    m_axiom = headerParts[0];
    m_angle = headerParts[1].toDouble();
    m_startAngle = headerParts[2].toDouble();
    qDebug() << "axiom:" << m_axiom << "angle:" << m_angle << "startAngle:" << m_startAngle;

    m_rules.clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList parts = line.split("->");
        if (parts.size() == 2) {
            QString left = parts[0].trimmed();
            if (!left.isEmpty()) {
                m_rules[left[0]] = parts[1].trimmed();
            }
        }
    }

    m_loaded = true;
    generate();
    update();
}

void LSystemWidget::setIterations(int n)
{
    m_iterations = n;
    generate();
}

void LSystemWidget::setRandomness(bool enabled)
{
    m_random = enabled;
    generate();
}

void LSystemWidget::generate()
{
    QString current = m_axiom;
    for (int i = 0; i < m_iterations; ++i) {
        QString next;
        next.reserve(current.size() * 2);
        for (QChar c : current) {
            if (m_rules.contains(c)) {
                next += m_rules[c];
            } else {
                next += c;
            }
        }
        current = next;
    }
    interpret(current);
}

void LSystemWidget::interpret(const QString &commandString)
{
    m_lines.clear();
    QStack<QPair<QPointF, double>> stack;
    QPointF currentPos(0, 0);
    double currentAngle = m_startAngle;
    double step = 10.0;
    int currentDepth = 0;
    for (QChar c : commandString) {
        if (c == 'F' || c == 'G' || c == 'X' || c == 'Y' || c == 'A' || c == 'B') {
            double angleRad = qDegreesToRadians(currentAngle);
            QPointF nextPos = currentPos + QPointF(step * qCos(angleRad), step * qSin(angleRad));
            m_lines.append({QLineF(currentPos, nextPos), currentDepth, 0.0});
            currentPos = nextPos;
        } else if (c == '+') {
            double angleChange = m_angle;
            if (m_random) {
                angleChange += (QRandomGenerator::global()->generateDouble() * 30.0 - 15.0);
            }
            currentAngle -= angleChange;
        } else if (c == '-') {
            double angleChange = m_angle;
            if (m_random) {
                angleChange += (QRandomGenerator::global()->generateDouble() * 30.0 - 15.0);
            }
            currentAngle += angleChange;
        } else if (c == '[') {
            stack.push({currentPos, currentAngle});
            currentDepth++;
        } else if (c == ']') {
            if (!stack.isEmpty()) {
                currentPos = stack.top().first;
                currentAngle = stack.top().second;
                stack.pop();
                currentDepth--;
            }
        }
    }
}

void LSystemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_lines.isEmpty()) return;

    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const LSystemLine &item : m_lines) {
        minX = qMin(minX, qMin(item.line.x1(), item.line.x2()));
        maxX = qMax(maxX, qMax(item.line.x1(), item.line.x2()));
        minY = qMin(minY, qMin(item.line.y1(), item.line.y2()));
        maxY = qMax(maxY, qMax(item.line.y1(), item.line.y2()));
    }

    double width = maxX - minX;
    double height = maxY - minY;
    if (width < 1e-6 || height < 1e-6) return;

    double margin = 20.0;
    double scaleX = (this->width() - 2 * margin) / width;
    double scaleY = (this->height() - 2 * margin) / height;
    double scale = qMin(scaleX, scaleY);

    double renderedW = width * scale;
    double renderedH = height * scale;
    double offsetX = (this->width() - renderedW) / 2.0;
    double offsetY = (this->height() - renderedH) / 2.0;

    for (const LSystemLine &item : m_lines) {
        auto toScreen = [&](double mx, double my) -> QPointF {
            double sx = offsetX + (mx - minX) * scale;
            double sy = offsetY + (maxY - my) * scale;
            return QPointF(sx, sy);
        };

        QPointF p1 = toScreen(item.line.x1(), item.line.y1());
        QPointF p2 = toScreen(item.line.x2(), item.line.y2());
        double penWidth = 2.0;
        QColor color = Qt::black;
        if (item.depth > 0) {
            penWidth = qMax(0.5, 4.0 * qPow(0.7, item.depth));
            int r = 139 - item.depth * 10;
            int g = 69 + item.depth * 20;
            int b = 19 + item.depth * 5;
            color = QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
        }

        painter.setPen(QPen(color, penWidth));
        painter.drawLine(p1, p2);
    }
}