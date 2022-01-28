#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H
#include "ESPAsyncWebServer.h"
class RequestHandler {
public:
    virtual ~RequestHandler() { }
    virtual bool canHandle(WebRequestMethod method, String uri) { (void) method; (void) uri; return false; }
    virtual bool canUpload(String uri) { (void) uri; return false; }
    virtual bool handle(ESP32WebServer& server, WebRequestMethod requestMethod, String requestUri) { (void) server; (void) requestMethod; (void) requestUri; return false; }
    virtual void upload(ESP32WebServer& server, String requestUri, HTTPUpload& upload) { (void) server; (void) requestUri; (void) upload; }

    RequestHandler* next() { return _next; }
    void next(RequestHandler* r) { _next = r; }

private:
    RequestHandler* _next = nullptr;
};

#endif //REQUESTHANDLER_H
