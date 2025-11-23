#pragma once
#include <string>
#include <vector>

enum class ProblemType {
    Evaluation,
    RootFinding,
    Simplification,
    CustomSolution
};

enum class UserRole {
    Student,
    Admin
};

struct User {
    int id = 0;
    std::string username;
    std::string password;
    UserRole role = UserRole::Student;
};

struct Problem {
    int id = 0;
    std::string title;
    std::string description;
    std::string difficulty;
    ProblemType type = ProblemType::Evaluation;
    std::string polyCoeffs;
    std::string solution;
};

struct Submission {
    int id = 0;
    int userId = 0;
    int problemId = 0;
    std::string userAnswer;
    bool isCorrect = false;
    double score = 0.0;
    std::string submittedAt;
    std::string correctAnswer;
};

struct CustomSolutionRequest {
    int id = 0;
    int userId = 0;
    std::string polynomial;
    std::string solution;
    std::string requestedAt;
};

ProblemType problemTypeFromString(const std::string& s);
std::string problemTypeToString(ProblemType t);
UserRole userRoleFromString(const std::string& s);
std::string userRoleToString(UserRole r);