#pragma once
#include <string>
#include <vector>
#include <utility>
#include <sqlext.h>  // ODBC
#include "Models.h"
#include "Exceptions.h"

class DbManager {
public:
    DbManager();
    ~DbManager();

    void connect(const std::string& dsn,
                 const std::string& user,
                 const std::string& password);
    void disconnect();

    User getUserByUsername(const std::string& username);
    User createUser(const std::string& username, const std::string& password);

    std::vector<Problem> getProblems();
    void insertSubmission(const Submission& s);

    std::vector<std::pair<std::string,double>> getLeaderboard();

    // Root cache
    std::vector<double> getCachedRoots(int problemId);
    void cacheRoots(int problemId, const std::vector<double>& roots);

private:
    SQLHENV  hEnv_;
    SQLHDBC  hDbc_;
    bool     connected_;

    void checkRet(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle,
                  const std::string& msg);
};
