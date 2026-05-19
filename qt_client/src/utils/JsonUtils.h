#pragma once

#include <QJsonObject>
#include <QString>

namespace JsonUtils {
QString readString(const QJsonObject &object, const QString &key);
}
