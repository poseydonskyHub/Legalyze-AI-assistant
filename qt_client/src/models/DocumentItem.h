#pragma once

#include <QString>

struct DocumentItem
{
    int id = 0;
    QString filename;
    QString extension;
    QString contentType;
    QString extractedVia;
    QString extractedText;
    QString analysisSummary;
    QString indexedInRag;
    QString createdAt;
};
