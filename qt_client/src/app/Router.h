#pragma once

#include <QObject>

class QStackedWidget;
class QWidget;

class Router : public QObject
{
    Q_OBJECT

public:
    explicit Router(QObject *parent = nullptr);

    void setStack(QStackedWidget *stack);
    void registerPage(const QString &name, QWidget *page);
    void showPage(const QString &name);

private:
    QStackedWidget *m_stack = nullptr;
};
