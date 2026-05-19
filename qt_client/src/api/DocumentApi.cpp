#include "api/DocumentApi.h"

DocumentApi::DocumentApi(ApiClient *client, QObject *parent)
    : QObject(parent),
      m_client(client)
{
}
