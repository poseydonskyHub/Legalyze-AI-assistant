#pragma once

#include <QWidget>

class MessageBubbleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBubbleWidget(QWidget *parent = nullptr);
};
