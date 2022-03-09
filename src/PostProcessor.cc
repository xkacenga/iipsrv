#include "PostProcessor.h"

#include <iostream>
#include <algorithm>

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

    if (contentType.find("multipart") != string::npos)
    {
        string boundary = getBoundary(contentType);
        map<string, string> mimeData = processMultipartMime(content, boundary);
        content = mimeData["protocol"] + "/" + mimeData["fileName"] + "/" + mimeData["json"];
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

string PostProcessor::getBoundary(string contentType)
{
    string boundaryIdentifier = "boundary=";
    int boundaryPos = contentType.find(boundaryIdentifier) + boundaryIdentifier.size();
    string boundary = contentType.substr(boundaryPos, contentType.size());
    return boundary;
}

map<string, string> PostProcessor::processMultipartMime(const string &mime,
                                                        const string &boundary)
{
    map<string, string> mimeData;

    vector<string> parts = getParts(mime, boundary);
    for (const string &part : parts)
    {
        vector<string> lines = getLines(part);
        string name = Utils::split(lines[0], "; name=\"").back();
        name.pop_back(); // remove " from the end of the name
        string data = lines.back();
        mimeData.insert(make_pair(name, data));
    }
    return mimeData;
}

vector<string> PostProcessor::getParts(const string &mime,
                                       const string &boundary)
{
    vector<string> parts = Utils::split(mime, "--" + boundary);
    parts.erase(remove_if(parts.begin(), parts.end(),
                          [](const string &s)
                          { return s.find("; name=") == string::npos; }),
                parts.end());
    return parts;
}

vector<string> PostProcessor::getLines(const string &part)
{
    vector<string> lines = Utils::split(part, "\r\n");
    lines.erase(remove_if(lines.begin(), lines.end(),
                          [](const string &s)
                          {
                              return s.find_first_not_of(" \t\n\v\f\r") == string::npos;
                          }),
                lines.end());
    return lines;
}
