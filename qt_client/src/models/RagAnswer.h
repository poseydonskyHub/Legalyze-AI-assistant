#pragma once

#include <QList>
#include <QString>

#include "models/Citation.h"

struct RagAnswer
{
    QString answer;
    QList<Citation> citations;
    double confidenceScore = 0.0;
    QString refusalReason;
    QString sourceValidation;
};
