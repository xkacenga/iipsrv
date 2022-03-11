#include "Task.h"
#include "Environment.h"
#include "Utils.h"

#include <cstdlib>
#include <fstream>
#include <algorithm>

#include <json/json.h>
#include <pqxx/pqxx>

using namespace std;

void sendJsonResponse(Session *session, const Json::Value &jsonRoot);

void Annotation::run(Session *session, const string &argument)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation handler reached" << std::endl;

    // Time this command
    if (session->loglevel >= 2)
        command_timer.start();

    string command = argument.substr(0, argument.find("/"));
    string args = argument.substr(argument.find("/") + 1, argument.size());

    if (command == "getList")
    {
        getList(session, args);
    }
    else if (command == "load")
    {
        vector<string> stringIds = Utils::split(args, ",");
        vector<int> ids;
        std::transform(stringIds.begin(), stringIds.end(), std::back_inserter(ids),
                       [](string s) -> int
                       { return atoi(s.c_str()); });
        load(session, ids);
    }
    else if (command == "save")
    {
        vector<string> splitArgs = Utils::split(args, ",", 3);
        save(session, splitArgs[0], splitArgs[1], splitArgs[2]);
    }

    if (session->loglevel >= 2)
    {
        *(session->logfile) << "Annotation :: Total command time " << command_timer.getTime() << " microseconds" << endl;
    }
}

void Annotation::getList(Session *session, const string &tissuePath)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: getList handler reached" << std::endl;

    pqxx::result result = Utils::executeNonTransaction(session, "getList", tissuePath);

    Json::Value root;
    root["tissuePath"] = tissuePath;
    root["annotations"] = Json::arrayValue;

    for (auto const &row : result)
    {
        Json::Value annotation;
        annotation["id"] = atoi(row[0].c_str());
        annotation["name"] = row[1].c_str();
        root["annotations"].append(annotation);
    }

    sendJsonResponse(session, root);
}

void Annotation::load(Session *session, const std::vector<int> &annotationIds)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: load handler reached" << std::endl;

    Json::Value root;
    root["annotations"] = Json::arrayValue;

    for (int id : annotationIds)
    {
        pqxx::result result = Utils::executeNonTransaction(session, "load", id);
        for (auto const &row : result)
        {
            string path = row[0].c_str();
            Json::Value annotation;
            ifstream jsonFile(path);
            if (jsonFile.good())
            {
                jsonFile >> annotation;
                root["annotations"].append(annotation);
            }
        }
    }
    sendJsonResponse(session, root);
}

void saveJsonFile(Session *session, string absPath, string jsonString);

void Annotation::save(Session *session, const string &tissuePath,
                      const string &jsonName, const string &jsonString)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: save handler reached" << std::endl;

    pqxx::result getTissueIdResult = Utils::executeNonTransaction(
        session, "getTissueIdAndAbsPath", tissuePath);

    string tissueAbsPath;
    int tissueId;
    if (getTissueIdResult.empty())
    {
        tissueAbsPath = Environment::getFileSystemPrefix() + tissuePath;
        ifstream tissueFile(tissueAbsPath);
        if (!tissueFile.good())
        {
            throw annotation_error("Tissue file does not exist!\n");
        }

        pqxx::result insertTissueResult = Utils::executeTransaction(
            session, "insertTissue", tissuePath, tissueAbsPath);
        tissueId = insertTissueResult[0][0].as<int>();
    }
    else
    {
        tissueId = getTissueIdResult[0][0].as<int>();
        tissueAbsPath = getTissueIdResult[0][1].c_str();
    }

    string annotFilePrefix = Environment::getAnnotFolder();
    string tissueName = Utils::getFileName(tissuePath);
    string newJsonName = tissueName + "_" + to_string(tissueId) + "-" + jsonName + ".json";
    string absJsonPath = annotFilePrefix + newJsonName;

    Utils::executeTransaction(
        session, "insertAnnotation", newJsonName, absJsonPath, tissueId);
    saveJsonFile(session, absJsonPath, jsonString);
    return;
}

void saveJsonFile(Session *session, string absPath, string jsonString) {
    ofstream outFile(absPath);

    Json::Value annotationRoot;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    bool parsingSuccessful = reader->parse(jsonString.c_str(),
                                           jsonString.c_str() + jsonString.size(),
                                           &annotationRoot, nullptr);
    if (!parsingSuccessful)
    {
        throw annotation_error("Error while parsing json!\n");
    }
    Json::StyledWriter styledWriter;
    outFile << styledWriter.write(annotationRoot);
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