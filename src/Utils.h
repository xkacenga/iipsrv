#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <Task.h>

class Utils
{
public:
    // from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    static inline std::vector<std::string> split(const std::string &argument,
                                                 const std::string &delimiter,
                                                 int limit = -1)
    {
        std::vector<std::string> strings;

        int counter = 1;
        auto start = 0U;
        auto end = argument.find(delimiter);
        while (end != std::string::npos)
        {
            if (counter == limit)
            {
                strings.push_back(argument.substr(start, argument.size() - 1));
                return strings;
            }
            strings.push_back(argument.substr(start, end - start));
            start = end + delimiter.length();
            end = argument.find(delimiter, start);
            counter++;
        }
        strings.push_back(argument.substr(start, end));
        return strings;
    }

    /**
     * @brief Gets file name whithout directory and suffix.
     *
     * @param filePath file path
     * @return file name
     */
    static inline std::string getFileName(const std::string &filePath)
    {
        std::string tissueName = filePath.substr(
            filePath.find_last_of('/') + 1, filePath.size() - 1);
        tissueName = tissueName.substr(0, tissueName.find_first_of('.'));
        return tissueName;
    }

    /**
     * @brief Adds '/' to the end of file path if needed.
     *
     * @param filePath file path
     * @return fixed file path
     */
    static inline std::string fixFilePath(const std::string &filePath)
    {
        std::string newPath = filePath;
        if (newPath.back() != '/')
        {
            newPath.push_back('/');
        }
        return newPath;
    }

    template <typename... Args>
    static pqxx::result executeNonTransaction(pqxx::connection * const connection, std::string query, Args &&...args)
    {
        pqxx::nontransaction nt(*connection);
        return nt.exec_prepared(query, &args...);
    }

    template <typename... Args>
    static pqxx::result executeTransaction(pqxx::connection * const connection, std::string query, Args &&...args)
    {
        pqxx::work t(*connection);
        pqxx::result result;
        try
        {
            result = t.exec_prepared(query, &args...);
            t.commit();
        }
        catch (const pqxx::pqxx_exception &e)
        {
            t.abort();
            throw annotation_error(e.base().what());
        }
        return result;
    }
};

#endif
