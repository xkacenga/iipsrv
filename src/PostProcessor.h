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
     * If it contains application/json request,
     * the method returns request string for 'save' or 'update' command.
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
};

#endif