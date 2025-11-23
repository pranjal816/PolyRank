#include "DbManager.h"
#include <iostream>
#include <stdexcept>
#include <vector>

DbManager::DbManager() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC), connected(false) {}

DbManager::~DbManager() { disconnect(); }

void DbManager::check(SQLRETURN ret, SQLSMALLINT type, SQLHANDLE handle, const std::string& msg) {
    if (SQL_SUCCEEDED(ret)) return;

    SQLCHAR state[6], msgText[256];
    SQLINTEGER native;
    SQLSMALLINT len;
    SQLGetDiagRec(type, handle, 1, state, &native, msgText, sizeof(msgText), &len);
    throw std::runtime_error(msg + ": " + (char*)msgText);
}

void DbManager::connect(const std::string& dsn, const std::string& user, const std::string& pass) {
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    std::cout << "Testing available ODBC drivers..." << std::endl;
    
    // Get available drivers first
    std::vector<std::string> availableDrivers;
    SQLCHAR driverDesc[256];
    SQLCHAR driverAttr[256];
    SQLSMALLINT descLen, attrLen;
    SQLUSMALLINT direction = SQL_FETCH_FIRST;
    
    while (SQL_SUCCEEDED(SQLDrivers(hEnv, direction, driverDesc, sizeof(driverDesc), &descLen, 
                                   driverAttr, sizeof(driverAttr), &attrLen))) {
        std::string driverName = (char*)driverDesc;
        availableDrivers.push_back(driverName);
        std::cout << "Found driver: " << driverName << std::endl;
        direction = SQL_FETCH_NEXT;
    }

    // Try MySQL drivers that might be available
    std::vector<std::string> driversToTry;
    
    // Check which MySQL drivers are actually available
    for (const auto& available : availableDrivers) {
        if (available.find("MySQL") != std::string::npos) {
            driversToTry.push_back("DRIVER={" + available + "};");
        }
    }
    
    // If no MySQL drivers found, try common ones
    if (driversToTry.empty()) {
        std::cout << "No MySQL drivers found in system. Trying common names..." << std::endl;
        driversToTry = {
            "DRIVER={MySQL ODBC 8.0 Unicode Driver};",
            "DRIVER={MySQL ODBC 8.0 ANSI Driver};",
            "DRIVER={MySQL ODBC 8.1 Unicode Driver};", 
            "DRIVER={MySQL ODBC 8.1 ANSI Driver};",
            "DRIVER={MySQL ODBC 8.2 Unicode Driver};",
            "DRIVER={MySQL ODBC 8.2 ANSI Driver};",
            "DRIVER={MySQL ODBC 8.3 Unicode Driver};",
            "DRIVER={MySQL ODBC 8.3 ANSI Driver};",
            "DRIVER={MySQL ODBC 8.4 Unicode Driver};",
            "DRIVER={MySQL ODBC 8.4 ANSI Driver};"
        };
    }

    SQLRETURN ret = SQL_ERROR;
    std::string errorMessages;
    
    for (const auto& driver : driversToTry) {
        std::string conn = driver +
                          "SERVER=127.0.0.1;"
                          "PORT=3306;"
                          "DATABASE=PolyRank;"
                          "USER=" + user + ";"
                          "PASSWORD=" + pass + ";"
                          "OPTION=3;";

        std::cout << "Trying: " << driver << std::endl;
        
        SQLCHAR out[1024];
        SQLSMALLINT outLen;
        ret = SQLDriverConnect(hDbc, nullptr, (SQLCHAR*)conn.c_str(), SQL_NTS,
                              out, sizeof(out), &outLen, SQL_DRIVER_NOPROMPT);
        
        if (SQL_SUCCEEDED(ret)) {
            connected = true;
            std::cout << "Connected successfully!" << std::endl;
            return;
        } else {
            SQLCHAR state[6], msgText[256];
            SQLINTEGER native;
            SQLSMALLINT len;
            SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, state, &native, msgText, sizeof(msgText), &len);
            errorMessages += "Driver " + driver + " failed: " + (char*)msgText + "\n";
        }
    }
    
    throw std::runtime_error("All connection attempts failed:\n" + errorMessages);
}

void DbManager::disconnect() {
    if (connected) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        connected = false;
    }
}

// ... REST OF YOUR DbManager.cpp METHODS REMAIN THE SAME ...
User DbManager::getUserByUsername(const std::string& username) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);
    std::string q = "SELECT user_id, username, password FROM Users WHERE username='" + username + "'";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "User lookup failed");

    User u;
    if (SQLFetch(stmt) == SQL_NO_DATA) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        throw std::runtime_error("User not found");
    }

    SQLGetData(stmt, 1, SQL_C_SLONG, &u.id, 0, nullptr);
    char uname[50], pass[50];
    SQLGetData(stmt, 2, SQL_C_CHAR, uname, sizeof(uname), nullptr);
    SQLGetData(stmt, 3, SQL_C_CHAR, pass, sizeof(pass), nullptr);
    u.username = uname;
    u.password = pass;

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return u;
}

User DbManager::createUser(const std::string& u, const std::string& p, UserRole role) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);
    
    std::string roleStr = userRoleToString(role);
    std::string q = "INSERT INTO Users (username, password, role) VALUES ('" + u + "','" + p + "','" + roleStr + "')";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "User insert failed");

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return getUserByUsername(u);
}

std::vector<Problem> DbManager::getProblems() {
    std::vector<Problem> v;
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)"SELECT problem_id,title,description,difficulty,type,poly_coeffs FROM Problems", SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Problem fetch failed");

    while (SQLFetch(stmt) != SQL_NO_DATA) {
        Problem p;
        SQLGetData(stmt, 1, SQL_C_SLONG, &p.id, 0, nullptr);
        char t[100], d[255], dif[20], tp[20], coeffs[200];
        SQLGetData(stmt, 2, SQL_C_CHAR, t, sizeof(t), nullptr);
        SQLGetData(stmt, 3, SQL_C_CHAR, d, sizeof(d), nullptr);
        SQLGetData(stmt, 4, SQL_C_CHAR, dif, sizeof(dif), nullptr);
        SQLGetData(stmt, 5, SQL_C_CHAR, tp, sizeof(tp), nullptr);
        SQLGetData(stmt, 6, SQL_C_CHAR, coeffs, sizeof(coeffs), nullptr);

        p.title = t;
        p.description = d;
        p.difficulty = dif;
        p.type = problemTypeFromString(tp);
        p.polyCoeffs = coeffs;
        v.push_back(p);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return v;
}

void DbManager::insertSubmission(const Submission& s) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string q = "INSERT INTO Submissions (user_id,problem_id,user_answer,is_correct,score,correct_answer) VALUES (" +
                   std::to_string(s.userId) + "," +
                   std::to_string(s.problemId) + ",'" +
                   s.userAnswer + "'," +
                   (s.isCorrect ? "1" : "0") + "," +
                   std::to_string(s.score) + ",'" +
                   s.correctAnswer + "')";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Insert submission failed");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

std::vector<double> DbManager::getCachedRoots(int problemId) {
    std::vector<double> roots;
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string q = "SELECT root_value FROM PolynomialSolutions WHERE problem_id=" + std::to_string(problemId);
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Root fetch failed");

    double v;
    while (SQLFetch(stmt) != SQL_NO_DATA) {
        SQLGetData(stmt, 1, SQL_C_DOUBLE, &v, 0, nullptr);
        roots.push_back(v);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return roots;
}

void DbManager::cacheRoots(int problemId, const std::vector<double>& roots) {
    for (size_t i = 0; i < roots.size(); ++i) {
        SQLHSTMT stmt;
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

        std::string q = "INSERT INTO PolynomialSolutions (problem_id,root_index,root_value) VALUES (" +
                       std::to_string(problemId) + "," +
                       std::to_string(i) + "," +
                       std::to_string(roots[i]) + ")";

        SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
        check(ret, SQL_HANDLE_STMT, stmt, "Cache root failed");
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }
}

void DbManager::insertCustomSolutionRequest(const CustomSolutionRequest& request) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string q = "INSERT INTO CustomSolutionRequests (user_id, polynomial, solution) VALUES (" +
                   std::to_string(request.userId) + ",'" +
                   request.polynomial + "','" +
                   request.solution + "')";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Custom solution request insert failed");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

std::vector<CustomSolutionRequest> DbManager::getCustomSolutionRequests(int userId) {
    std::vector<CustomSolutionRequest> requests;
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string query = "SELECT request_id, user_id, polynomial, solution, requested_at FROM CustomSolutionRequests";
    if (userId > 0) {
        query += " WHERE user_id = " + std::to_string(userId);
    }
    query += " ORDER BY requested_at DESC";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Get custom solutions failed");

    while (SQLFetch(stmt) != SQL_NO_DATA) {
        CustomSolutionRequest req;
        SQLGetData(stmt, 1, SQL_C_SLONG, &req.id, 0, nullptr);
        SQLGetData(stmt, 2, SQL_C_SLONG, &req.userId, 0, nullptr);

        char polynomial[255], solution[255], requestedAt[50];
        SQLGetData(stmt, 3, SQL_C_CHAR, polynomial, sizeof(polynomial), nullptr);
        SQLGetData(stmt, 4, SQL_C_CHAR, solution, sizeof(solution), nullptr);
        SQLGetData(stmt, 5, SQL_C_CHAR, requestedAt, sizeof(requestedAt), nullptr);

        req.polynomial = polynomial;
        req.solution = solution;
        req.requestedAt = requestedAt;
        requests.push_back(req);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return requests;
}

std::vector<Submission> DbManager::getUserSubmissions(int userId) {
    std::vector<Submission> submissions;
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string query = "SELECT submission_id, user_id, problem_id, user_answer, is_correct, score, correct_answer, submitted_at FROM Submissions WHERE user_id = " + std::to_string(userId) + " ORDER BY submitted_at DESC";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Get user submissions failed");

    while (SQLFetch(stmt) != SQL_NO_DATA) {
        Submission s;
        SQLGetData(stmt, 1, SQL_C_SLONG, &s.id, 0, nullptr);
        SQLGetData(stmt, 2, SQL_C_SLONG, &s.userId, 0, nullptr);
        SQLGetData(stmt, 3, SQL_C_SLONG, &s.problemId, 0, nullptr);

        char userAnswer[255], correctAnswer[255], submittedAt[50];
        SQLGetData(stmt, 4, SQL_C_CHAR, userAnswer, sizeof(userAnswer), nullptr);
        SQLGetData(stmt, 5, SQL_C_SLONG, &s.isCorrect, 0, nullptr);
        SQLGetData(stmt, 6, SQL_C_DOUBLE, &s.score, 0, nullptr);
        SQLGetData(stmt, 7, SQL_C_CHAR, correctAnswer, sizeof(correctAnswer), nullptr);
        SQLGetData(stmt, 8, SQL_C_CHAR, submittedAt, sizeof(submittedAt), nullptr);

        s.userAnswer = userAnswer;
        s.correctAnswer = correctAnswer;
        s.submittedAt = submittedAt;
        submissions.push_back(s);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return submissions;
}

std::vector<std::pair<std::string, double>> DbManager::getLeaderboard() {
    std::vector<std::pair<std::string, double>> leaderboard;
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string query = "SELECT u.username, SUM(s.score) as total_score "
                       "FROM Users u JOIN Submissions s ON u.user_id = s.user_id "
                       "WHERE s.is_correct = 1 "
                       "GROUP BY u.user_id, u.username "
                       "ORDER BY total_score DESC";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Leaderboard fetch failed");

    char username[50];
    double score;
    while (SQLFetch(stmt) != SQL_NO_DATA) {
        SQLGetData(stmt, 1, SQL_C_CHAR, username, sizeof(username), nullptr);
        SQLGetData(stmt, 2, SQL_C_DOUBLE, &score, 0, nullptr);
        leaderboard.push_back({username, score});
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return leaderboard;
}