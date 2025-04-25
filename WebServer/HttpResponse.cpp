#include "HttpResponse.h"
#include "HttpRequest.h"



void HttpResponse::UpdateResponse(const HttpRequest& Request)
{
	statusCode = NONE;
	statusMessage.clear();
	headers.clear();
	body.clear();
	protocol = Request.getProtocol();
    checkResponse(Request);
    setHeaders(Request);


}

void HttpResponse::checkResponse(const HttpRequest& Request)
{
    switch (Request.getMethod())
    {
    case GET:
        getRequest(Request);
        break;
    case POST:
        postRequest(Request);
        break;
    case PUT:
        putRequest(Request);
        break;
    case DELETE:
        deleteRequest(Request);
        break;
    case HEAD:
        headRequest(Request);
        break;
    case TRACE:
        traceRequest(Request);
        break;
    case OPTIONS:
        optionsRequest(Request);
        break;
    default:
        //The server does not support the request type
        setStatus(METHOD_NOT_ALLOWED, "", "");
        break;
    }
}

// Function to get the full file path based on the request URL and language parameter
string HttpResponse::getFilePath(const HttpRequest& request) {
    string basePath = "C:/temp"; // Base directory for files
    string filePath = basePath + request.getUrl(); // Combine base path and URL

    // Check for the language parameter and adjust file path accordingly
    string lang = request.getQueries().count("lang") ? request.getQueries().at("lang") : "en"; // Default to English
    if (lang == "he") {
        filePath = basePath + "/he" + request.getUrl();
    }
    else if (lang == "fr") {
        filePath = basePath + "/fr" + request.getUrl();
    }

    return filePath;
}

// Function to handle GET requests
void HttpResponse::getRequest(const HttpRequest& request) {
    // Find the full file path using a helper function
    string filePath = getFilePath(request);

    
    if (!fileExists(filePath))
    {
        setStatus(NOT_FOUND,filePath,"");
        return;
    }
    //Attempt to open the file for reading
    ifstream infile;
    infile.open(filePath, ios_base::in);

    //If there is an error opening file
    if (!infile.is_open())
    {
        setStatus(INTERNAL_ERROR, filePath, "open");
        return;
    }

    string content((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());


    // Close the file
    infile.close();

    //If there is an error closing the file
    if (infile.is_open())
    {
        setStatus(INTERNAL_ERROR, filePath, "close");
        return;
    }


    // Set status to OK after successfully reading the file
    setStatus(OK, filePath, "");

    // Set the response body to the content of the file
    body = content;

}

void HttpResponse::setStatus(int code, const string& fileName, const string& fileState){
    statusCode = code; // Set the status code

    // Set the status message based on the code
    switch (code) {
    case OK:
        statusCode = OK;
        statusMessage = "OK";
        break;
    case BAD_REQUEST:
        statusCode = BAD_REQUEST;
        statusMessage = "Bad Request";
        break;
    case NO_CONTENT:
        statusCode = NO_CONTENT;
        statusMessage = "No Content"; 
        break;
    case NOT_FOUND:
        statusCode = NOT_FOUND;
        body = "The file was not found.";
        statusMessage = "Not Found";
        cerr << "The file was not found:" << fileName << endl;
        break;
    case CONFLICT:
        statusCode = CONFLICT;
        statusMessage = "Conflict";
        body = "The request could not be completed due to a conflict with the current state of the resource.";
        cerr << "Conflict occurred: " << fileName << endl;
        break;
    case CREATED:
        statusCode = CREATED;
        statusMessage = "Created";
        body = "The resource has been created successfully.";
        break;
    case FORBIDDEN:
        statusCode = FORBIDDEN;
        statusMessage = "Forbidden";
        break;
    case INTERNAL_ERROR:
        handleInternalError(fileName, fileState);
        break;
    case METHOD_NOT_ALLOWED :
        statusCode = METHOD_NOT_ALLOWED;
        statusMessage = "Method Not Allowed";
        body = "The method used is not allowed for the requested resource.";
        headers["Allow"] = "GET, POST, PUT, DELETE, HEAD, TRACE, OPTIONS";
        cerr << "Method Not Allowed"<< endl;
        break;
    default:
        statusMessage = "Unknown Status";
        break;
    }
}

void HttpResponse::handleInternalError(const string& fileName, const string& fileState)
{
    statusCode = INTERNAL_ERROR; // Set the status code to 500
    statusMessage = "Internal Server Error"; // Set the status message

    // Determine the error message based on the file state
    if (fileState == "open") {
        body = "Failed to open the file: " + fileName;
        cerr << "Failed to open the file: " << fileName << endl;
    }
    else if (fileState == "close") {
        body = "Failed to close the file: " + fileName;
        cerr << "Failed to close the file: " << fileName << endl;
    }
    else if (fileState == "delete") {
        body = "Failed to delete the file: " + fileName;
        cerr << "Failed to delete the file: " << fileName << endl;
    }
    else if (fileState == "write") {
        body = "Failed to write to file: " + fileName;
        cerr << "Failed to write to file: " << fileName << endl;
    }
    else {
        body = "An unknown error occurred with the file: " + fileName;
    }


    // Log the error to the standard error stream
    cerr << "Error 500: " << body << endl;
}

// Check if a file exists
bool HttpResponse::fileExists(const string& filename)
{
    ifstream infile(filename);
    return infile.good();
}

void HttpResponse::postRequest(const HttpRequest& request) {
    // Extract the file path from the request
    string filePath = getFilePath(request);

    //If file is already exits
    if (fileExists(filePath))
    {
        setStatus(CONFLICT, filePath, "");
        return;
    }


    ofstream outfile(filePath, ios::app); // Open the file in append mode
    if (!outfile.is_open()) {
        // If the file cannot be opened, handle the error
        setStatus(INTERNAL_ERROR, filePath, "open");
        return;
    }

    // Write the body content to the file
    outfile << request.getBody(); // Append the request body to the file

    // Close the file after writing
    outfile.close();

    // Check if the file closed successfully
    if (outfile.is_open()) {
        // Handle the error if the file did not close properly
        setStatus(INTERNAL_ERROR, filePath, "close");
        return;
    }

    // Print the body content to the console as required
    cout << "Content written to the file: " << body << endl;

    // Update the status to indicate the operation was successful
    setStatus(CREATED, filePath, "");
}


void HttpResponse::putRequest(const HttpRequest& request) {
    // Extract the file path from the request
    string filePath = getFilePath(request);


    //check if the file exits (for a Status Number)
    bool fileExisted = fileExists(filePath);

    // Attempt to open the file in write mode (this will overwrite existing content)
    ofstream outfile(filePath, ios::out); // Open the file in write mode
    if (!outfile.is_open()) {
        // If the file cannot be opened, handle the error
        setStatus(INTERNAL_ERROR, filePath, "open");
        return;
    }

    // Write the body content to the file, replacing its current content
    outfile << request.getBody();

    // Check for write errors
    if (outfile.fail()) {
        setStatus(INTERNAL_ERROR, filePath, "write");
        outfile.close();
        return;
    }

    // Close the file after writing
    outfile.close();

    // Check if the file closed successfully
    if (outfile.is_open()) {
        // Handle the error if the file did not close properly
        setStatus(INTERNAL_ERROR, filePath, "close");
        return;
    }

    // Determine response code based on whether the file existed before
    if (!fileExisted)
    {
        // Update the status to indicate the operation was successful
        setStatus(CREATED, filePath, "");
    }
    else
    {
        // Update the status to indicate the operation was successful
        setStatus(OK, filePath, "");
        body = "The file was updated successfully.";
    }


}

void HttpResponse::deleteRequest(const HttpRequest& request) {
    // Extract the file path from the request
    string filePath = getFilePath(request);

    // Check if the file exists
    if (!fileExists(filePath)) {
        setStatus(NOT_FOUND, filePath, "");
        return;
    }

    // Attempt to delete the file
    if (remove(filePath.c_str()) != 0) {
        // If there was an error deleting the file
        setStatus(INTERNAL_ERROR, filePath, "delete");
        return;
    }

    // If the file was deleted successfully
    setStatus(NO_CONTENT, filePath, "");
    cout << "File deleted successfully: " << filePath << endl;
}

void HttpResponse::traceRequest(const HttpRequest& request) {
    // Set the response status to OK (200)
    setStatus(OK,"","");

    // Create a stringstream to store the mirrored request
    stringstream responseBody;

    // Add the request line to the response 
    responseBody << "TRACE" << " " << request.getUrl() << " " << request.getProtocol() << "\r\n";


    // Add all headers from the request to the response
    for (const auto& header : request.getHeaders()) {
        responseBody << header.first << ": " << header.second << "\r\n";
    }

    // Add a blank line to separate headers from the body
    responseBody << "\r\n";

    // Add the body of the request, if any
    responseBody << request.getBody();

    // Set the mirrored request as the response body
    body = responseBody.str();

}

// Handle OPTIONS request
void HttpResponse::optionsRequest(const HttpRequest& request) {

    // Set the response status to NO_CONTENT (204)
    setStatus(NO_CONTENT, "", "");
    headers["Allow"] = "GET, POST, PUT, DELETE, HEAD, TRACE, OPTIONS";
}

void HttpResponse::headRequest(const HttpRequest& request) {
    // Find the full file path using a helper function
    string filePath = getFilePath(request);

    // Check if the file exists
    if (!fileExists(filePath)) {
        setStatus(NOT_FOUND, filePath, "");
        return;
    }

    // Attempt to open the file
    ifstream infile(filePath, ios::in);
    if (!infile.is_open()) {
        setStatus(INTERNAL_ERROR, filePath, "open");
        return;
    }

    // Get the file size and set headers accordingly
    infile.seekg(0, ios::end); // Move to the end of the file
    size_t fileSize = infile.tellg(); // Get the size of the file
    infile.close();

    // Set the response status to OK
    setStatus(OK,"","");

    // Set the headers without returning the file content
    headers["Content-Length"] = to_string(fileSize);
}


void HttpResponse::setHeaders(const HttpRequest& request) {

    // Set the Content-Type to text/html, as required by the assignment
    headers["Content-Type"] = "text/html";

    // Set the Content-Length header
    if (request.getMethod() != HEAD || statusCode == NOT_FOUND)
    {
        headers["Content-Length"] = to_string(body.length());
    }
}

// Function to format the HttpResponse into a response string
string HttpResponse::getResponse() const {
    stringstream responseStream;

    // Add the status line: Protocol Status-Code Status-Message
    responseStream << protocol << " " << statusCode << " " << statusMessage << "\r\n";

    // Add headers
    for (const auto& header : headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
    }

    // Add a blank line between headers and body
    responseStream << "\r\n";

    // Add the body content
    responseStream << body;

    // Convert the stringstream to a string and return
    return responseStream.str();
}