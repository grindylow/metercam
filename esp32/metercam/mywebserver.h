#ifndef webserver_h
#define webserver_h

#include <ESPAsyncWebServer.h>
#include "camoperator.h"

void init_webserver(AsyncWebServer & s, CamOperator * o);

#endif
