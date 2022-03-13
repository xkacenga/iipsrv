#ifndef _POST_PROCESSOR_H
#define _POST_PROCESSOR_H

#include <fcgiapp.h>

#include <string>
#include <map>

#include "Logger.h"
#include "Utils.h"

class PostProcessor
{
public:
    PostProcessor(int loglevel, Logger *logfile) : loglevel(loglevel),
                                                   logfile(logfile){};

    /**
     * @brief Processes POST request.
     * If contains multipart request, the method returns request string for 'save' command.
     * 
     * @param request fcgi request
     * @return std::string request string containing command and arguments
     */
    std::string process(const FCGX_Request &request);

private:
    int loglevel;
    Logger *logfile;

    int getContentLength(const FCGX_Request &request);
    std::string getContent(const FCGX_Request &request, int contentLength);
    std::string getContentType(const FCGX_Request &request);
    std::string getBoundary(std::string contentType);

    std::map<std::string, std::string> processMultipartMime(const std::string &mime,
                                                            const std::string &boundary);
    std::vector<std::string> getParts(const std::string &mime,
                                      const std::string &boundary);
    std::vector<std::string> getLines(const std::string &part);
};

#endif