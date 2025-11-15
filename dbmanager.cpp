#include "DbManager.h"
#include <iostream>

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

    std::string conn = "DSN=" + dsn + ";UID=" + user + ";PWD=" + pass + ";";
    SQLCHAR out[1024];
    SQLSMALLINT outLen;

    SQLRETURN ret = SQLDriverConnect(hDbc, nullptr, (SQLCHAR*)conn.c_str(), SQL_NTS,
                                     out, sizeof(out), &outLen, SQL_DRIVER_NOPROMPT);

    check(ret, SQL_HANDLE_DBC, hDbc, "DB Connect failed");
    connected = true;
}

void DbManager::disconnect() {
    if (connected) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

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

User DbManager::createUser(const std::string& u, const std::string& p) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string q = "INSERT INTO Users (username, password) VALUES ('" + u + "','" + p + "')";
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "User insert failed");

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return getUserByUsername(u);
}



void DbManager::insertSubmission(const Submission& s) {
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    std::string q = "INSERT INTO Submissions (user_id,problem_id,user_answer,is_correct,score) VALUES (" +
        std::to_string(s.userId) + "," +
        std::to_string(s.problemId) + ",'" +
        s.userAnswer + "'," +
        (s.isCorrect ? "1" : "0") + "," +
        std::to_string(s.score) + ")";

    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    check(ret, SQL_HANDLE_STMT, stmt, "Insert submission failed");

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}
