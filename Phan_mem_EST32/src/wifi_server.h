#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

void setupWiFiAndWeb();
void cleanupWebSocket();
void sendTelemetryToWeb();
int getConnectedDevices();

#endif