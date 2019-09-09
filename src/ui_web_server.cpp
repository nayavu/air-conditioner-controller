//
// Created by naya.vu on 11.09.2019.
//

#include "ui_web_server.h"

bool EncodedStaticRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri)  {
    server.sendHeader("Content-Encoding", _enc);
    return StaticRequestHandler::handle(server, requestMethod, requestUri);
}

void UploadableStaticRequestHandler::upload(ESP8266WebServer &server, String requestUri, HTTPUpload &upload) {
    HTTPUpload& upl = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        fsUploadFile = SPIFFS.open(_path, "w");
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
            server.send(200);
        } else {
            server.send(500);
        }
    }
}

UiWebServer::UiWebServer() : ESP8266WebServer() {
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/", "/index.html.br", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/index.html", "/index.html.br", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/favicon.ico", "/favicon.ico", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/main.js", "/main.js.br", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/polyfills.js", "/polyfills.js.br", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/runtime.js", "/runtime.js.br", "br", NULL));
    addHandler(new EncodedStaticRequestHandler(SPIFFS, "/styles.css", "/styles.css.br", "br", NULL));
}

void UiWebServer::serveConfig(const char *uri, const char *path) {
    addHandler(new UploadableStaticRequestHandler(SPIFFS, uri, path));
}