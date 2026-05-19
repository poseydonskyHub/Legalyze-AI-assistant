#include "network/NetworkUtils.h"

namespace NetworkUtils {
QString bearerHeaderValue(const QString &token)
{
    return QString("Bearer %1").arg(token);
}
}
