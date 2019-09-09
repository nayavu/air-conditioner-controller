//
// Created by naya.vu on 11.09.2019.
//

#ifndef REMOTE_CONTROL_UI_WEB_SERVER_H
#define REMOTE_CONTROL_UI_WEB_SERVER_H


#include <ESP8266WebServer.h>
#include <FS.h>
#include "detail/RequestHandlersImpl.h"

class EncodedStaticRequestHandler : public StaticRequestHandler {
public:
    EncodedStaticRequestHandler(FS& fs, const char* path, const char* uri, const char* enc, const char* cache_header)
            : StaticRequestHandler(fs, path, uri, cache_header), _enc(enc) { };

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override ;
protected:
    String _enc;
};

class UploadableStaticRequestHandler : public StaticRequestHandler {
public:
    UploadableStaticRequestHandler(FS& fs, const char* path, const char* uri, const char* cache_header = NULL)
            : StaticRequestHandler(fs, path, uri, cache_header) { };

    bool canUpload(String uri) override { return true; }
    void upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload) override;

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
    UiWebServer();
    void serveConfig(const char *uri, const char *path);
};


#endif //REMOTE_CONTROL_UI_WEB_SERVER_H
