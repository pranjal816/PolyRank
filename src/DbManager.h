#pragma once
#include <windows.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include "Models.h"

class DbManager {
public:
    DbManager();
    ~DbManager();

    void connect(const std::string& dsn, const std::string& user, const std::string& pass);
    void disconnect();

    User getUserByUsername(const std::string& username);
    User createUser(const std::string& username, const std::string& pass, UserRole role = UserRole::Student);
    
    std::vector<Problem> getProblems();
    Problem createProblem(const Problem& problem);
    
    void insertSubmission(const Submission& s);
    std::vector<std::pair<std::string, double>> getLeaderboard();
    
    std::vector<double> getCachedRoots(int problemId);
    void cacheRoots(int problemId, const std::vector<double>& roots);
    
    void insertCustomSolutionRequest(const CustomSolutionRequest& request);
    std::vector<CustomSolutionRequest> getCustomSolutionRequests(int userId = 0);
    std::vector<Submission> getUserSubmissions(int userId);

private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool connected;

    void check(SQLRETURN ret, SQLSMALLINT type, SQLHANDLE handle, const std::string& msg);
};