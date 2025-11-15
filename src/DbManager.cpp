#include "DbManager.h"
#include <iostream>

DbManager::DbManager()
    : hEnv_(SQL_NULL_HENV),
      hDbc_(SQL_NULL_HDBC),
      connected_(false) {}

DbManager::~DbManager() {
    disconnect();
}

void DbManager::checkRet(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle,
                         const std::string& msg) {
    if (SQL_SUCCEEDED(ret)) return;

    SQLCHAR sqlState[6], message[256];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;
    SQLGetDiagRec(handleType, handle, 1, sqlState, &nativeError,
                  message, sizeof(message), &textLength);
    throw DbException(msg + " | " + reinterpret_cast<char*>(message));
}

void DbManager::connect(const std::string& dsn,
                        const std::string& user,
                        const std::string& password) {
    if (connected_) return;

    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv_);
    checkRet(ret, SQL_HANDLE_ENV, hEnv_, "Alloc ENV failed");

    ret = SQLSetEnvAttr(hEnv_, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    checkRet(ret, SQL_HANDLE_ENV, hEnv_, "Set ODBC version failed");

    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv_, &hDbc_);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc DBC failed");

    std::string connStr = "DSN=" + dsn + ";UID=" + user + ";PWD=" + password + ";";
    SQLCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    ret = SQLDriverConnect(hDbc_, nullptr,
                           (SQLCHAR*)connStr.c_str(), SQL_NTS,
                           outConnStr, sizeof(outConnStr),
                           &outConnStrLen,
                           SQL_DRIVER_NOPROMPT);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Connection failed");
    connected_ = true;
}

void DbManager::disconnect() {
    if (connected_) {
        SQLDisconnect(hDbc_);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc_);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);
        hDbc_ = SQL_NULL_HDBC;
        hEnv_ = SQL_NULL_HENV;
        connected_ = false;
    }
}

User DbManager::getUserByUsername(const std::string& username) {
    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query = "SELECT user_id, username, password FROM Users WHERE username = ?";
    ret = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Prepare failed");

    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                           0, 0, (SQLPOINTER)username.c_str(), username.size(), nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind param failed");

    ret = SQLExecute(hStmt);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Execute failed");

    User u;
    SQLINTEGER id;
    char uname[64], pwd[64];
    SQLLEN cbId, cbU, cbP;

    ret = SQLFetch(hStmt);
    if (ret == SQL_NO_DATA) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw DbException("User not found");
    }
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Fetch failed");

    SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
    SQLGetData(hStmt, 2, SQL_C_CHAR, uname, sizeof(uname), &cbU);
    SQLGetData(hStmt, 3, SQL_C_CHAR, pwd, sizeof(pwd), &cbP);

    u.id = id;
    u.username = uname;
    u.password = pwd;

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return u;
}

User DbManager::createUser(const std::string& username, const std::string& password) {
    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query = "INSERT INTO Users (username, password) VALUES (?, ?)";
    ret = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Prepare failed");

    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                           0, 0, (SQLPOINTER)username.c_str(), username.size(), nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind username failed");

    ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                           0, 0, (SQLPOINTER)password.c_str(), password.size(), nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind password failed");

    ret = SQLExecute(hStmt);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Execute failed");

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    return getUserByUsername(username);
}

std::vector<Problem> DbManager::getProblems() {
    std::vector<Problem> problems;

    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query =
        "SELECT problem_id, title, description, difficulty, type, poly_coeffs "
        "FROM Problems";
    ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "ExecDirect failed");

    while (SQLFetch(hStmt) != SQL_NO_DATA) {
        Problem p;
        SQLINTEGER id;
        char title[128], desc[256], diff[32], type[32], coeffs[256];
        SQLLEN cb;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cb);
        SQLGetData(hStmt, 2, SQL_C_CHAR, title, sizeof(title), &cb);
        SQLGetData(hStmt, 3, SQL_C_CHAR, desc, sizeof(desc), &cb);
        SQLGetData(hStmt, 4, SQL_C_CHAR, diff, sizeof(diff), &cb);
        SQLGetData(hStmt, 5, SQL_C_CHAR, type, sizeof(type), &cb);
        SQLGetData(hStmt, 6, SQL_C_CHAR, coeffs, sizeof(coeffs), &cb);

        p.id = id;
        p.title = title;
        p.description = desc;
        p.difficulty = diff;
        p.type = problemTypeFromString(type);
        p.polyCoeffs = coeffs;

        problems.push_back(p);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return problems;
}

void DbManager::insertSubmission(const Submission& s) {
    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query =
        "INSERT INTO Submissions (user_id, problem_id, user_answer, is_correct, score) "
        "VALUES (?, ?, ?, ?, ?)";

    ret = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Prepare failed");

    SQLINTEGER userId = s.userId;
    SQLINTEGER problemId = s.problemId;
    SQLINTEGER isCorrect = s.isCorrect ? 1 : 0;
    double score = s.score;

    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                           0, 0, &userId, 0, nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind user_id failed");

    ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                           0, 0, &problemId, 0, nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind problem_id failed");

    ret = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                           0, 0, (SQLPOINTER)s.userAnswer.c_str(),
                           s.userAnswer.size(), nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind answer failed");

    ret = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                           0, 0, &isCorrect, 0, nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind is_correct failed");

    ret = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE,
                           0, 0, &score, 0, nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind score failed");

    ret = SQLExecute(hStmt);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Execute failed");

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

std::vector<std::pair<std::string,double>> DbManager::getLeaderboard() {
    std::vector<std::pair<std::string,double>> lb;

    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query =
        "SELECT u.username, SUM(s.score) AS total_score "
        "FROM Users u "
        "JOIN Submissions s ON u.user_id = s.user_id "
        "GROUP BY u.username "
        "ORDER BY total_score DESC";

    ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "ExecDirect failed");

    while (SQLFetch(hStmt) != SQL_NO_DATA) {
        char uname[64];
        double totalScore;
        SQLLEN cb;

        SQLGetData(hStmt, 1, SQL_C_CHAR, uname, sizeof(uname), &cb);
        SQLGetData(hStmt, 2, SQL_C_DOUBLE, &totalScore, 0, &cb);

        lb.emplace_back(uname, totalScore);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return lb;
}

// --------- Root caching methods ----------

std::vector<double> DbManager::getCachedRoots(int problemId) {
    std::vector<double> roots;
    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
    checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

    std::string query =
        "SELECT root_value FROM PolynomialSolutions "
        "WHERE problem_id = ? ORDER BY root_index ASC";

    ret = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Prepare failed");

    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                           0, 0, &problemId, 0, nullptr);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind param failed");

    ret = SQLExecute(hStmt);
    checkRet(ret, SQL_HANDLE_STMT, hStmt, "Execute failed");

    double rootVal;
    SQLLEN cb;
    while (SQLFetch(hStmt) != SQL_NO_DATA) {
        SQLGetData(hStmt, 1, SQL_C_DOUBLE, &rootVal, 0, &cb);
        roots.push_back(rootVal);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return roots;
}

void DbManager::cacheRoots(int problemId, const std::vector<double>& roots) {
    for (int i = 0; i < static_cast<int>(roots.size()); ++i) {
        SQLHSTMT hStmt;
        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt);
        checkRet(ret, SQL_HANDLE_DBC, hDbc_, "Alloc STMT failed");

        std::string query =
            "INSERT INTO PolynomialSolutions (poly_id, problem_id, root_index, root_value) "
            "VALUES (?, ?, ?, ?)";

        ret = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Prepare failed");

        int polyId = problemId;
        int idx = i + 1;
        double val = roots[i];

        ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                               0, 0, &polyId, 0, nullptr);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind poly_id failed");

        ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                               0, 0, &problemId, 0, nullptr);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind problem_id failed");

        ret = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                               0, 0, &idx, 0, nullptr);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind root_index failed");

        ret = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE,
                               0, 0, &val, 0, nullptr);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Bind root_value failed");

        ret = SQLExecute(hStmt);
        checkRet(ret, SQL_HANDLE_STMT, hStmt, "Execute failed");

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
}
