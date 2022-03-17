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
        (*logfile) << "content:\n"
                   << content << endl;
        map<string, string> mimeData = processMultipartMime(content, boundary);
        content = formRequestString(mimeData);
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

string PostProcessor::getBoundary(const string &contentType)
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
        string name = getName(lines[0]);
        string data = getData(lines);

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

string PostProcessor::getName(const string &line)
{
    size_t namePos = line.find("; name=\"");
    if (namePos == string::npos)
    {
        return "";
    }
    string name = line.substr(namePos + 8);
    size_t quotationPos = name.find("\"");
    name = name.substr(0, quotationPos);
    return name;
}

string PostProcessor::getData(vector<string> &lines)
{
    string data;
    if (lines.size() >= 2 && lines[1] == "Content-Type: application/json")
    {
        lines.erase(lines.begin(), lines.begin() + 2);
    }
    else
    {
        lines.erase(lines.begin(), lines.begin() + 1);
    }

    for (const string &line : lines)
    {
        data += line;
    }
    return Utils::trim(data);
}

string PostProcessor::formRequestString(const map<string, string> &mimeData)
{
    string request;
    if (mimeData.at("command") == "save")
    {
        request = mimeData.at("protocol") + "=" + mimeData.at("command") +
                  "/" + mimeData.at("tissuePath") +
                  "," + mimeData.at("name") + "," + mimeData.at("data");
    }
    else if (mimeData.at("command") == "update")
    {
        request = mimeData.at("protocol") + "=" + mimeData.at("command") +
                  "/" + mimeData.at("id") + "," + mimeData.at("data");
    }
    return request;
}
