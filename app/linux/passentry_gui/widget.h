#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QThread>
#include "polling.h"

class Widget : public QWidget
{
    Q_OBJECT
    QThread pollingThread;
public:
    Widget(QWidget *parent = 0);
    ~Widget();
    QLineEdit *ple;
private:
    void Interior();
public slots:
    void cmd(const QString &result);
signals:
    void poll();
};
#endif // WIDGET_H
