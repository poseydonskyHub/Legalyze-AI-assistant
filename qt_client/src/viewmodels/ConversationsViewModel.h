#pragma once

#include <QObject>

class ConversationsViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ConversationsViewModel(QObject *parent = nullptr);
};
