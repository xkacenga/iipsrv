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

    if (command == "load")
    {
        vector<string> stringIds = Utils::split(args, ",");
        vector<int> ids;
        std::transform(stringIds.begin(), stringIds.end(), std::back_inserter(ids),
                       [](string s) -> int
                       { return atoi(s.c_str()); });
        load(session, ids);
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

    pqxx::nontransaction nt(*(session->connection));
    pqxx::result result = nt.exec_prepared("getList", tissuePath);

    Json::Value root;
    root["tissuePath"] = tissuePath;
    root["annotations"] = Json::arrayValue;

    for (auto const &row : result)
    {
        Json::Value annotation;
        annotation["id"] = atoi(row[0].c_str());
        annotation["annotationPath"] = row[1].c_str();
        root["annotations"].append(annotation);
    }

    sendJsonResponse(session, root);
}

void Annotation::load(Session *session, const std::vector<int> &annotationIds)
{
    if (session->loglevel >= 3)
        (*session->logfile) << "Annotation:: load handler reached" << std::endl;

    pqxx::nontransaction nt(*(session->connection));

    Json::Value root;
    root["annotations"] = Json::arrayValue;

    for (int id : annotationIds)
    {
        pqxx::result result = nt.exec_prepared("load", id);
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

void save(Session *session) {
    return;
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