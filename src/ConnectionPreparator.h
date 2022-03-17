#ifndef _CONNECTION_PREPARATOR_H
#define _CONNECTION_PREPARATOR_H

#include <string>
#include <pqxx/pqxx>

class ConnectionPreparator
{
public:
    /**
     * @brief Prepares sql queries.
     *
     * @param connection connection
     * @return true if connection successfully opened
     * @return false otherwise
     */
    static bool prepare(pqxx::connection &connection)
    {
        using namespace std;

        string listAnnotations = "SELECT a.id, a.name \
                       FROM tissues AS t INNER JOIN annotations AS a \
                       ON t.id = a.tissue_id \
                       WHERE t.path = $1";
        connection.prepare("listAnnotations", listAnnotations);

        string getAnnotationData = "SELECT data \
                    FROM annotations \
                    WHERE id = $1";
        connection.prepare("getAnnotationData", getAnnotationData);

        string getTissueIdAndAbsPath = "SELECT id, abs_path \
                          FROM tissues \
                          WHERE path = $1";
        connection.prepare("getTissueIdAndAbsPath", getTissueIdAndAbsPath);

        string insertTissue = "INSERT INTO tissues (path, abs_path) \
                         VALUES ($1, $2) \
                         RETURNING id";
        connection.prepare("insertTissue", insertTissue);

        string insertAnnotation = "INSERT INTO annotations (name, data, tissue_id) \
                             VALUES ($1, $2, $3)";
        connection.prepare("insertAnnotation", insertAnnotation);

        string deleteAnnotation = "DELETE FROM annotations \
                                   WHERE id = $1";
        connection.prepare("deleteAnnotation", deleteAnnotation);

        string updateAnnotation = "UPDATE annotations \
                                   SET data = $1 \
                                   WHERE id = $2";
        connection.prepare("updateAnnotation", updateAnnotation);
        
        return connection.is_open();
    };
};

#endif