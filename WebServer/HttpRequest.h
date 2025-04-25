#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

#define NONE 0
#define GET 1
#define HEAD 2
#define POST 3
#define PUT 4
#define DELETE 5
#define TRACE 6
#define OPTIONS 7


class HttpRequest
{
private:
    int method;                   // GET, POST, etc.
    string url;                      // URL from the request, e.g., /index.html
    map<string, string> queries; // Key-value pairs for  queries
    string protocol;                  // HTTP version, e.g., HTTP/1.1
    map<string,string> headers; // Key-value pairs for headers
    string body;                     // Body content, if any

    // Helper function to split a string by a delimiter
    vector<string> split(const string& str, const string& delimiter);

    // Helper function to trim whitespace from a string
    string trim(const string& str);

public:

    // Constructor
       HttpRequest() : method(NONE), url(""), protocol(""), body("") {}


    // Function to parse the raw HTTP request
    void parseRequest(const string& rawRequest);
    string getProtocol() const { return protocol; }
    int getMethod() const { return method; }
    string getUrl() const { return url; }
    string getBody() const { return body; }
    map<string, string> getQueries() const { return queries; }
    map<string, string> getHeaders() const { return headers; }

};
