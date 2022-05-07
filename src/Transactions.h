#ifndef _TRANSACTIONS_H
#define _TRANSACTIONS_H

#include <pqxx/pqxx>
#include <string>

class Transactions
{
public:
    /**
     * @brief Executes Postgre non-transaction (i.e. select).
     *
     * @tparam Args
     * @param connection pointer to pqxx connection
     * @param query string defining prepared statement
     * @param args query arguments
     * @return pqxx::result
     */
    template <typename... Args>
    static pqxx::result executeNonTransaction(
        pqxx::connection *const connection, std::string query, Args &&...args)
    {
        pqxx::nontransaction nt(*connection);
        return nt.exec_prepared(query, &args...);
    }

    /**
     * @brief Executes Postgre transaction.
     *
     * @tparam Args
     * @param connection pointer to pqxx connection
     * @param query string defining prepared statement
     * @param args query arguments
     * @return pqxx::result
     */
    template <typename... Args>
    static pqxx::result executeTransaction(
        pqxx::connection *const connection, std::string query, Args &&...args)
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