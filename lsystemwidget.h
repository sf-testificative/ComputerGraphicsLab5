#ifndef LSYSTEMWIDGET_H
#define LSYSTEMWIDGET_H

#include <QWidget>
#include <QMap>
#include <QVector>
#include <QPointF>
#include <QStack>
#include <QColor>

struct LSystemLine {
    QLineF line;
    int depth;
};

class LSystemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LSystemWidget(QWidget *parent = nullptr);

    void loadFromFile(const QString &filename);
    void setRandomness(bool enabled);
    void setIterations(int n);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_axiom;
    double m_angle = 60.0;
    double m_startAngle = 90.0;
    QMap<QChar, QString> m_rules;
    QVector<LSystemLine> m_lines;
    int m_iterations = 4;
    bool m_random = false;
    bool m_loaded = false;

    void generate();
    void interpret(const QString &s);
};

#endif