//
// Created by naya.vu on 11.09.2019.
//

#ifndef REMOTE_CONTROL_UIWEBSERVER_H
#define REMOTE_CONTROL_UIWEBSERVER_H


#include <ESP8266WebServer.h>
#include <FS.h>

/**
 * A subclass of ESP8266WebServer with pre-configured routes for Angular application
 * It assumes, that JS/HTML files are gzipped
 */
class UiWebServer : public ESP8266WebServer {
private:

public:
    UiWebServer() : ESP8266WebServer() { };
    void begin() override;
};


#endif //REMOTE_CONTROL_UIWEBSERVER_H
