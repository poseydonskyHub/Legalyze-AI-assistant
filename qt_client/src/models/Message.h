#pragma once

#include <QString>

struct Message
{
    int id = 0;
    QString role;
    QString content;
    QString createdAt;
};
