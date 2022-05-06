#include "Task.h"
#include "Environment.h"
#include "Utils.h"
#include "Transactions.h"

#include <cstdio>
#include <algorithm>

#include <jsoncpp/json/json.h>
#include <pqxx/pqxx>

using namespace std;

void sendJsonResponse(Session *session, const Json::Value &jsonRoot);
int parseIdString(string idString);
void sendSuccessResponse(Session *session);

void Annotation::run(Session *session, const string &argument)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation handler reached" << std::endl;

    // Time this command
    if (session->loglevel >= 2)
        command_timer.start();

    if (!session->connection || !session->connection->is_open())
    {
        throw annotation_error("There is no open connection to the db!\n");
    }

    size_t slashPosition = argument.find("/");
    string command = argument.substr(0, slashPosition);
    string args = argument.substr(slashPosition + 1, argument.size());
    if (slashPosition == string::npos || args == "")
    {
        throw annotation_error("No arguments were specified for annotation command!\n");
    }

    if (command == "list")
    {
        list(session, args);
    }
    else if (command == "load")
    {
        int id = parseIdString(args);
        load(session, id);
    }
    else if (command == "save")
    {
        vector<string> splitArgs = Utils::split(args, ",", 2);
        save(session, splitArgs[0], splitArgs[1]);
    }
    else if (command == "update")
    {
        vector<string> splitArgs = Utils::split(args, ",", 2);
        int id = parseIdString(splitArgs[0]);
        update(session, id, splitArgs[1]);
    }
    else if (command == "remove")
    {
        int id = parseIdString(args);
        remove(session, id);
    }
    else
    {
        throw annotation_error(command + " is a wrong command!\n");
    }

    if (session->loglevel >= 2)
    {
        *(session->logfile) << "Annotation :: Total command time " << command_timer.getTime() << " microseconds" << endl;
    }
}

int parseIdString(string idString)
{
    int id;
    try
    {
        id = stoi(idString);
    }
    catch (const exception &e)
    {
        throw annotation_error("Conversion of id " + idString + " failed!\n");
    }
    return id;
}

void Annotation::list(Session *session, const string &tissuePath)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: list handler reached" << std::endl;

    pqxx::result result = Transactions::executeNonTransaction(
        session->connection, "listAnnotations", tissuePath);

    Json::Value root;
    root["tissuePath"] = tissuePath;
    root["annotations"] = Json::arrayValue;

    for (auto const &row : result)
    {
        Json::Value annotation;
        annotation["id"] = stoi(row[0].c_str());

        Json::Value data = Utils::parseJson(row[1].c_str());
        annotation["metadata"] = data["metadata"];
        root["annotations"].append(annotation);
    }

    sendJsonResponse(session, root);
}

void Annotation::load(Session *session, int annotationId)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: load handler reached" << std::endl;

    pqxx::result result = Transactions::executeNonTransaction(
        session->connection, "getAnnotationData", annotationId);

    Json::Value annotation;
    if (!result.empty())
    {
        string data = result[0][0].c_str();
        annotation = Utils::parseJson(data);
    }
    sendJsonResponse(session, annotation);
}

void Annotation::save(Session *session, const string &tissuePath, const string &data)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: save handler reached" << std::endl;

    Json::Value annotationRoot = Utils::parseJson(data);

    pqxx::result getTissueIdResult = Transactions::executeNonTransaction(
        session->connection, "getTissueIdAndAbsPath", tissuePath);

    string tissueAbsPath;
    int tissueId;
    if (getTissueIdResult.empty())
    {
        tissueAbsPath = Environment::getFileSystemPrefix() + tissuePath +
                        Environment::getFileSystemSuffix();
        ifstream tissueFile(tissueAbsPath);
        if (!tissueFile.good())
        {
            throw annotation_error(tissuePath + " does not exist!\n");
        }

        pqxx::result insertTissueResult = Transactions::executeTransaction(
            session->connection, "insertTissue", tissuePath, tissueAbsPath);
        tissueId = insertTissueResult[0][0].as<int>();
    }
    else
    {
        tissueId = getTissueIdResult[0][0].as<int>();
        tissueAbsPath = getTissueIdResult[0][1].c_str();
    }

    string tissueName = Utils::getFileName(tissuePath);
    string jsonData = Utils::jsonToString(annotationRoot);

    Transactions::executeTransaction(
        session->connection, "insertAnnotation", jsonData, tissueId);

    sendSuccessResponse(session);
    return;
}

void Annotation::update(Session *session, int annotationId, const string &data)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: update handler reached" << std::endl;

    Json::Value annotationRoot = Utils::parseJson(data);
    string jsonData = Utils::jsonToString(annotationRoot);

    Transactions::executeTransaction(
        session->connection, "updateAnnotation", annotationId, jsonData);
    sendSuccessResponse(session);
}

void Annotation::remove(Session *session, int annotationId)
{
    Transactions::executeTransaction(
        session->connection, "deleteAnnotation", annotationId);
    sendSuccessResponse(session);
}

void sendSuccessResponse(Session *session)
{
    Json::Value responseRoot;
    responseRoot["success"] = true;
    sendJsonResponse(session, responseRoot);
}

void sendJsonResponse(Session *session, const Json::Value &jsonRoot)
{
    string json = jsonRoot.toStyledString();
    stringstream header;
    header << session->response->createHTTPHeader(
        "json", "");
    if (session->out->putStr((const char *)header.str().c_str(), header.tellp()) == -1)
    {
        if (session->loglevel >= 1)
        {
            *(session->logfile) << "Annotation :: Error writing HTTP header" << endl;
        }
    }
    session->out->putStr((const char *)json.c_str(), json.size());
    session->response->setImageSent();
}