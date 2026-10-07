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
    double r;
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
    double m_angle;
    double m_startAngle;
    QMap<QChar, QString> m_rules;
    QVector<LSystemLine> m_lines;
    int m_iterations;
    bool m_random;
    bool m_loaded;

    void generate();
    void interpret(const QString &commandString);
};

#endif // LSYSTEMWIDGET_H