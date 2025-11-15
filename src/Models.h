#pragma once
#include <string>

enum class ProblemType {
    Evaluation,
    RootFinding,
    Simplification
};

struct User {
    int id = 0;
    std::string username;
    std::string password;
};

