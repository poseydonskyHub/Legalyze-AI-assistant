#include "utils/JsonUtils.h"

namespace JsonUtils {
QString readString(const QJsonObject &object, const QString &key)
{
    return object.value(key).toString();
}
}
