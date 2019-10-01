//
// Created by naya.vu on 11.09.2019.
//
#include "UiWebServer.h"

void UiWebServer::begin() {
    ESP8266WebServer::begin();
    serveStatic("/index.html", SPIFFS, "/index.html");
    serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
    serveStatic("/main.js", SPIFFS, "/main.js");
    serveStatic("/polyfills.js", SPIFFS, "/polyfills.js");
    serveStatic("/runtime.js", SPIFFS, "/runtime.js");
    serveStatic("/styles.css", SPIFFS, "/styles.css");
    serveStatic("/", SPIFFS, "/index.html");
}
