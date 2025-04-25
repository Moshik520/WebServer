#pragma once
#include "HttpRequest.h"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

// Define status codes
#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define FORBIDDEN 403
#define INTERNAL_ERROR 500
#define NO_CONTENT 204
#define CONFLICT 409
#define CREATED 201
#define METHOD_NOT_ALLOWED 405

class HttpResponse
{
private:

    int statusCode;                     // HTTP Status code
    string statusMessage;               // HTTP Status message
    string protocol;                    // Protocol version
    map<string, string> headers;        // Headers key-value pairs
    string body;                        // The content of the response

public:
    void UpdateResponse(const HttpRequest& Request);
    void checkResponse(const HttpRequest& Request);
    string getFilePath(const HttpRequest& request);
    void getRequest(const HttpRequest& request);
    void setStatus(int code, const string& fileName, const string& fileState);
    void handleInternalError(const string& fileName, const string& fileState);
    bool fileExists(const string& filename);
    void postRequest(const HttpRequest& request);
    void putRequest(const HttpRequest& request);
    void deleteRequest(const HttpRequest& request);
    void traceRequest(const HttpRequest& request);
    void optionsRequest(const HttpRequest& request);
    void headRequest(const HttpRequest& request);
    void setHeaders(const HttpRequest& request);
    string getResponse() const;

};

