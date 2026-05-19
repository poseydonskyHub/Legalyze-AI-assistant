#pragma once

#include <QObject>

class KnowledgeBaseViewModel : public QObject
{
    Q_OBJECT

public:
    explicit KnowledgeBaseViewModel(QObject *parent = nullptr);
};
