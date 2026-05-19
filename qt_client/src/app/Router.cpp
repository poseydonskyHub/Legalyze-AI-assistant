#include "app/Router.h"

#include <QMap>
#include <QStackedWidget>
#include <QWidget>

namespace {
QMap<QString, QWidget *> &pages()
{
    static QMap<QString, QWidget *> map;
    return map;
}
}

Router::Router(QObject *parent)
    : QObject(parent)
{
}

void Router::setStack(QStackedWidget *stack)
{
    m_stack = stack;
}

void Router::registerPage(const QString &name, QWidget *page)
{
    pages().insert(name, page);
    if (m_stack != nullptr && m_stack->indexOf(page) == -1) {
        m_stack->addWidget(page);
    }
}

void Router::showPage(const QString &name)
{
    if (m_stack == nullptr || !pages().contains(name)) {
        return;
    }
    m_stack->setCurrentWidget(pages().value(name));
}
