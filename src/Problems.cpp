#include "Problems.h"
#include "DbManager.h"
#include "PolynomialSolver.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cctype>

// ---------------- EvaluationProblem ----------------

EvaluationProblem::EvaluationProblem(const Problem& p)
    : PolynomialProblem(p) {
    // For demo, still use ID-based logic
    if (p.id == 1) {
        // p(x) = x^2 - 4, x = 2, expected = 0
        poly_ = Polynomial<double>({ -4.0, 0.0, 1.0 }); // -4 + 0x + 1x^2
        x_ = 2.0;
        expected_ = 0.0;
    } else {
        poly_ = Polynomial<double>({ 1.0 });
        x_ = 0.0;
        expected_ = 1.0;
    }
}

std::string EvaluationProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Enter numeric value of p(" << x_ << "): ";
    return oss.str();
}

bool EvaluationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    try {
        double ans = std::stod(userAnswer);
        double diff = std::fabs(ans - expected_);
        if (diff <= 1e-3) {
            score = 10.0;
            return true;
        }
        score = 0.0;
        return false;
    } catch (...) {
        throw InvalidPolynomialException("Could not parse numeric answer.");
    }
}

// ---------------- RootFindingProblem ----------------

static Polynomial<double> parsePolyFromCoeffs(const std::string& coeffsStr) {
    // coeffsStr: "-4,0,1" -> { -4, 0, 1 }
    std::vector<double> coeffs;
    std::stringstream ss(coeffsStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (token.empty()) continue;
        coeffs.push_back(std::stod(token));
    }
    if (coeffs.empty()) {
        // default p(x) = x^2 - 4
        coeffs = { -4.0, 0.0, 1.0 };
    }
    return Polynomial<double>(coeffs);
}

RootFindingProblem::RootFindingProblem(const Problem& p, DbManager& db)
    : PolynomialProblem(p) {

    poly_ = parsePolyFromCoeffs(p.polyCoeffs);

    // Try to get cached roots from DB
    roots_ = db.getCachedRoots(p.id);

    if (roots_.empty()) {
        // No cached roots -> solve using Newton-Raphson and cache
        roots_ = PolynomialSolver::solveNewton(poly_);

        if (!roots_.empty()) {
            db.cacheRoots(p.id, roots_);
        }
    }
}

std::string RootFindingProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Enter one real root of the polynomial: ";
    return oss.str();
}

bool RootFindingProblem::checkAnswer(const std::string& userAnswer, double& score) {
    if (roots_.empty()) {
        // If solver completely failed, treat as unsolvable
        throw InvalidPolynomialException("Could not solve polynomial roots.");
    }

    try {
        double ans = std::stod(userAnswer);
        bool match = false;
        for (double r : roots_) {
            if (std::fabs(ans - r) <= 1e-3) {
                match = true;
                break;
            }
        }
        if (match) {
            score = 10.0;
            return true;
        }
        score = 0.0;
        return false;
    } catch (...) {
        throw InvalidPolynomialException("Could not parse numeric root.");
    }
}

// ---------------- SimplificationProblem ----------------

SimplificationProblem::SimplificationProblem(const Problem& p)
    : PolynomialProblem(p) {

    if (p.id == 3) {
        expectedCanonical_ = "x^2+2x+1";
    } else {
        expectedCanonical_ = "x"; // fallback example
    }
}

std::string SimplificationProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Enter simplified polynomial (e.g., x^2+2x+1): ";
    return oss.str();
}

static std::string normalizePolyString(std::string s) {
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool SimplificationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    std::string expectedNorm = normalizePolyString(expectedCanonical_);
    std::string userNorm = normalizePolyString(userAnswer);
    if (expectedNorm == userNorm) {
        score = 10.0;
        return true;
    }
    score = 0.0;
    return false;
}

// ---------------- Factory ----------------

std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p, DbManager& db) {
    switch (p.type) {
    case ProblemType::Evaluation:
        return std::make_unique<EvaluationProblem>(p);
    case ProblemType::RootFinding:
        return std::make_unique<RootFindingProblem>(p, db);
    case ProblemType::Simplification:
        return std::make_unique<SimplificationProblem>(p);
    default:
        return std::make_unique<EvaluationProblem>(p);
    }
}
