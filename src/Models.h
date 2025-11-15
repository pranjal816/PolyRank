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

struct Problem {
    int id = 0;
    std::string title;
    std::string description;
    std::string difficulty;
    ProblemType type = ProblemType::Evaluation;
    std::string polyCoeffs;
};

struct Submission {
    int id = 0;
    int userId = 0;
    int problemId = 0;
    std::string userAnswer;
    bool isCorrect = false;
    double score = 0.0;
    std::string submittedAt;
};

ProblemType problemTypeFromString(const std::string& s);
std::string problemTypeToString(ProblemType t);
