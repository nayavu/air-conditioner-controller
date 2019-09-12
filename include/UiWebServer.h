//
// Created by naya.vu on 11.09.2019.
//

#ifndef REMOTE_CONTROL_UIWEBSERVER_H
#define REMOTE_CONTROL_UIWEBSERVER_H


#include <ESP8266WebServer.h>
#include <FS.h>
#include "detail/RequestHandlersImpl.h"

class PostableStaticRequestHandler : public StaticRequestHandler {
public:
    PostableStaticRequestHandler(FS& fs, const char* path, const char* uri, const char* cache_header = NULL)
            : StaticRequestHandler(fs, path, uri, cache_header) { };

    bool canHandle(HTTPMethod requestMethod, String requestUri) override;
    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override;

protected:
    File fsUploadFile;
};

/**
 * A subclass of ESP8266WebServer with pre-configured routes for Angular application
 * It assumes, that static files are encoded via brotli algorithm
 */
class UiWebServer : public ESP8266WebServer {
private:

public:
    UiWebServer() : ESP8266WebServer() { };
    void setupAngularApp();
    void serveUploadableFile(const char *uri, const char *path);
};


#endif //REMOTE_CONTROL_UIWEBSERVER_H
