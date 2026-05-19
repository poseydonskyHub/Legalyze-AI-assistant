#pragma once

#include <QObject>

class LoginViewModel : public QObject
{
    Q_OBJECT

public:
    explicit LoginViewModel(QObject *parent = nullptr);
};
