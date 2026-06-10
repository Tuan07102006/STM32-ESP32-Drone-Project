#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

void setupUDP();
void sendTelemetryUDP();
void receiveCommandsUDP(); 
void checkFailsafe();
#endif