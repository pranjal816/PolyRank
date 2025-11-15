#include "Models.h"

ProblemType problemTypeFromString(const std::string& s) {
    if (s == "EVAL") return ProblemType::Evaluation;
    if (s == "ROOT") return ProblemType::RootFinding;
    if (s == "SIMPLIFY") return ProblemType::Simplification;
    return ProblemType::Evaluation;
}
