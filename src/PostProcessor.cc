#include "PostProcessor.h"

#include <iostream>
#include <algorithm>

#include <Task.h>
#include <Utils.h>

using namespace std;

string PostProcessor::process(const FCGX_Request &request)
{
    if (loglevel >= 2)
    {
        (*logfile) << "Processing POST request" << endl;
    }

    int contentLength = getContentLength(request);
    string content = getContent(request, contentLength);
    string contentType = getContentType(request);

    if (contentType.find("application/json") != string::npos)
    {
        Json::Value request = Utils::parseJson(content);
        string protocol = request["protocol"].asString();
        string command = request["command"].asString();
        stringstream contentStream;
        if (command == "save")
        {
            string tissuePath = request["tissuePath"].asString();
            string data = Utils::jsonToString(request["data"]);
            contentStream << protocol << "=" << command << "/"
                          << tissuePath << "," << data;
        }
        else if (command == "update")
        {
            int annotationId = request["id"].asInt();
            string data = Utils::jsonToString(request["data"]);
            contentStream << protocol << "=" << command << "/"
                          << annotationId << "," << data;
        }
        content = contentStream.str();
    }
    return content;
}

int PostProcessor::getContentLength(const FCGX_Request &request)
{
    char *contentLengthString = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    int contentLength;
    if (contentLengthString)
    {
        contentLength = atoi(contentLengthString);
    }
    else
    {
        contentLength = 0;
    }
    return contentLength;
}

string PostProcessor::getContent(const FCGX_Request &request, int contentLength)
{
    char *contentBuffer = new char[contentLength];
    FCGX_GetStr(contentBuffer, contentLength, request.in);
    string content(contentBuffer, contentLength);
    delete[] contentBuffer;
    return content;
}

string PostProcessor::getContentType(const FCGX_Request &request)
{
    char *contentTypeString = FCGX_GetParam("CONTENT_TYPE", request.envp);
    string contentType(contentTypeString);
    return contentType;
}
