#include "Models.h"
#include <algorithm>

ProblemType problemTypeFromString(const std::string& s) {
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "EVAL") return ProblemType::Evaluation;
    if (upper == "ROOT") return ProblemType::RootFinding;
    if (upper == "SIMPLIFY") return ProblemType::Simplification;
    if (upper == "CUSTOM") return ProblemType::CustomSolution;
    return ProblemType::Evaluation;
}

std::string problemTypeToString(ProblemType t) {
    switch (t) {
        case ProblemType::Evaluation: return "EVAL";
        case ProblemType::RootFinding: return "ROOT";
        case ProblemType::Simplification: return "SIMPLIFY";
        case ProblemType::CustomSolution: return "CUSTOM";
        default: return "EVAL";
    }
}

UserRole userRoleFromString(const std::string& s) {
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "ADMIN") return UserRole::Admin;
    return UserRole::Student;
}

std::string userRoleToString(UserRole r) {
    switch (r) {
        case UserRole::Admin: return "ADMIN";
        case UserRole::Student: return "STUDENT";
        default: return "STUDENT";
    }
}