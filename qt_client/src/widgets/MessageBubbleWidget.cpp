#include "widgets/MessageBubbleWidget.h"

#include <QLabel>
#include <QVBoxLayout>

MessageBubbleWidget::MessageBubbleWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Message bubble", this));
}
