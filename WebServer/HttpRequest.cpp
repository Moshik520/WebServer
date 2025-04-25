#include "HttpRequest.h"

// Helper function to split a string by a delimiter
vector<string> HttpRequest::split(const string& str, const string& delimiter)
{
    vector<string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

// Helper function to trim whitespace from a string
string HttpRequest::trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// Function to parse the raw HTTP request
void HttpRequest::parseRequest(const string& rawRequest) 
{
    method = NONE;
    url.clear();
    queries.clear();
    protocol.clear();
    headers.clear();
    body.clear();
    // Split the raw request into lines based on "\r\n"
    vector<string> lines = split(rawRequest, "\r\n");

    // Parse the first line (method, url, protocol)
    if (!lines.empty()) 
    {
        vector<string> firstLine = split(lines[0], " ");
        if (firstLine.size() == 3) 
        {
            // Determine the method
            if (firstLine[0] == "GET") method = GET;
            else if (firstLine[0] == "POST") method = POST;
            else if (firstLine[0] == "HEAD") method = HEAD;
            else if (firstLine[0] == "PUT") method = PUT;
            else if (firstLine[0] == "DELETE") method = DELETE;
            else if (firstLine[0] == "TRACE") method = TRACE;
            else if (firstLine[0] == "OPTIONS") method = OPTIONS;
            else method = NONE;  // Default if method is not recognized

            url = firstLine[1];
            protocol = firstLine[2];
        }
    }

    // Parse headers (until an empty line)
    size_t i = 1;
    while (i < lines.size() && !lines[i].empty()) 
    {
        size_t delimiter = lines[i].find(": ");
        if (delimiter != string::npos) {
            string key = lines[i].substr(0, delimiter);
            string value = lines[i].substr(delimiter + 2);
            headers[trim(key)] = trim(value);
        }
        i++;
    }

    // Parse the body if it exists after an empty line
    if (i + 1 < lines.size()) {
        body = lines[i + 1];
    }

    // Parse queries from the URL if present
    size_t queryPos = url.find("?");
    if (queryPos != string::npos) {
        string queryStr = url.substr(queryPos + 1);
        url = url.substr(0, queryPos); // Update url without query
        vector<string> queryPairs = split(queryStr, "&");
        for (const string& query : queryPairs) {
            size_t eqPos = query.find("=");
            if (eqPos != string::npos) {
                string key = query.substr(0, eqPos);
                string value = query.substr(eqPos + 1);
                queries[trim(key)] = trim(value);
            }
        }
    }
}