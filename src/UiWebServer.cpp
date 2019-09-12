//
// Created by naya.vu on 11.09.2019.
//
#include "UiWebServer.h"

bool PostableStaticRequestHandler::canHandle(HTTPMethod requestMethod, String requestUri) {
    return requestMethod == HTTP_POST ? true : StaticRequestHandler::canHandle(requestMethod, requestUri);
}

bool PostableStaticRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
    if (requestMethod != HTTP_POST) {
        return StaticRequestHandler::handle(server, requestMethod, requestUri);
    }
    fsUploadFile = SPIFFS.open(_path, "w");
    if (fsUploadFile) {
        String content = server.arg("plain");
        fsUploadFile.write(content.c_str(), content.length());
        fsUploadFile.close();
        server.send(200);
    } else {
        server.send(500);
    }
    return true;
}

void UiWebServer::setupAngularApp() {
    serveStatic("/index.html", SPIFFS, "/index.html");
    serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
    serveStatic("/main.js", SPIFFS, "/main.js");
    serveStatic("/polyfills.js", SPIFFS, "/polyfills.js");
    serveStatic("/runtime.js", SPIFFS, "/runtime.js");
    serveStatic("/styles.css", SPIFFS, "/styles.css");
    serveStatic("/", SPIFFS, "/index.html");
}

void UiWebServer::serveUploadableFile(const char *uri, const char *path) {
    addHandler(new PostableStaticRequestHandler(SPIFFS, path, uri));
}