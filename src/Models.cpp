#include "Models.h"

ProblemType problemTypeFromString(const std::string& s) {
    if (s == "EVAL") return ProblemType::Evaluation;
    if (s == "ROOT") return ProblemType::RootFinding;
    if (s == "SIMPLIFY") return ProblemType::Simplification;
    return ProblemType::Evaluation; // default fallback
}

std::string problemTypeToString(ProblemType t) {
    switch (t) {
    case ProblemType::Evaluation:     return "EVAL";
    case ProblemType::RootFinding:    return "ROOT";
    case ProblemType::Simplification: return "SIMPLIFY";
    default:                          return "EVAL";
    }
}
