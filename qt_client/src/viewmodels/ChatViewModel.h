#pragma once

#include <QObject>

class ChatViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ChatViewModel(QObject *parent = nullptr);
};
