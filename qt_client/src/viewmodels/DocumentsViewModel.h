#pragma once

#include <QObject>

class DocumentsViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DocumentsViewModel(QObject *parent = nullptr);
};
