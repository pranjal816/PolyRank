#include "Problems.h"
#include "DbManager.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>

// Evaluation Problem
EvaluationProblem::EvaluationProblem(const Problem& p) : PolynomialProblem(p) {
    poly_ = Polynomial<double>::parse(p.polyCoeffs);
    x_ = 2.0;
    expected_ = poly_.evaluate(x_);
}

std::string EvaluationProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Polynomial: " << poly_.toString() << "\n"
        << "Compute p(" << x_ << "): ";
    return oss.str();
}

bool EvaluationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    try {
        double ans = std::stod(userAnswer);
        double diff = std::fabs(ans - expected_);
        if (diff < 1e-3) {
            score = 10;
            return true;
        }
    } catch (const std::exception&) {
        // Invalid input
    }
    score = 0;
    return false;
}

std::string EvaluationProblem::getSolution() const {
    std::ostringstream oss;
    oss << "The value of p(" << x_ << ") is " << expected_;
    return oss.str();
}

std::string EvaluationProblem::getCorrectAnswer() const {
    return std::to_string(expected_);
}

// Root Finding Problem
RootFindingProblem::RootFindingProblem(const Problem& p, DbManager& db) 
    : PolynomialProblem(p) {
    poly_ = Polynomial<double>::parse(p.polyCoeffs);
    roots_ = db.getCachedRoots(p.id);
    if (roots_.empty()) {
        roots_ = PolynomialSolver<double>::solveNewton(poly_);
        db.cacheRoots(p.id, roots_);
    }
}

std::string RootFindingProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Polynomial: " << poly_.toString() << "\n"
        << "Enter a real root: ";
    return oss.str();
}

bool RootFindingProblem::checkAnswer(const std::string& userAnswer, double& score) {
    try {
        double ans = std::stod(userAnswer);
        for (double r : roots_) {
            if (std::fabs(ans - r) < 1e-3) {
                score = 10;
                return true;
            }
        }
    } catch (const std::exception&) {
        // Invalid input
    }
    score = 0;
    return false;
}

std::string RootFindingProblem::getSolution() const {
    std::ostringstream oss;
    oss << "The roots of the polynomial are: ";
    for (size_t i = 0; i < roots_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << roots_[i];
    }
    return oss.str();
}

std::string RootFindingProblem::getCorrectAnswer() const {
    if (roots_.empty()) return "No real roots";
    return std::to_string(roots_[0]);
}

// Simplification Problem
SimplificationProblem::SimplificationProblem(const Problem& p) 
    : PolynomialProblem(p) {
    expected_ = "x^2+2x+1";
    polynomial_ = "(x+1)^2";
}

std::string SimplificationProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Simplify: " << polynomial_ << "\n"
        << "Enter simplified form (no spaces): ";
    return oss.str();
}

bool SimplificationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    std::string user = userAnswer;
    user.erase(std::remove_if(user.begin(), user.end(), ::isspace), user.end());
    std::transform(user.begin(), user.end(), user.begin(), ::tolower);
    
    std::string exp = expected_;
    std::transform(exp.begin(), exp.end(), exp.begin(), ::tolower);
    
    if (user == exp) {
        score = 10;
        return true;
    }
    score = 0;
    return false;
}

std::string SimplificationProblem::getSolution() const {
    return "The simplified form is: " + expected_;
}

std::string SimplificationProblem::getCorrectAnswer() const {
    return expected_;
}

// Custom Solution Problem
CustomSolutionProblem::CustomSolutionProblem(const Problem& p, DbManager& db)
    : PolynomialProblem(p) {
    poly_ = Polynomial<double>::parse(p.polyCoeffs);
    roots_ = PolynomialSolver<double>::solveNewton(poly_);
    
    std::ostringstream oss;
    oss << "Polynomial: " << poly_.toString() << "\n";
    oss << "Degree: " << poly_.degree() << "\n";
    oss << "Roots: ";
    for (size_t i = 0; i < roots_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << roots_[i];
    }
    solution_ = oss.str();
}

std::string CustomSolutionProblem::getPrompt() const {
    return "Enter the polynomial you want to solve (comma-separated coefficients): ";
}

bool CustomSolutionProblem::checkAnswer(const std::string& userAnswer, double& score) {
    // For custom problems, we don't check answers - we provide solutions
    score = 0;
    return false;
}

std::string CustomSolutionProblem::getSolution() const {
    return solution_;
}

std::string CustomSolutionProblem::getCorrectAnswer() const {
    return solution_;
}

// Factory function
std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p, DbManager& db) {
    switch (p.type) {
        case ProblemType::Evaluation:
            return std::make_unique<EvaluationProblem>(p);
        case ProblemType::RootFinding:
            return std::make_unique<RootFindingProblem>(p, db);
        case ProblemType::Simplification:
            return std::make_unique<SimplificationProblem>(p);
        case ProblemType::CustomSolution:
            return std::make_unique<CustomSolutionProblem>(p, db);
        default:
            return std::make_unique<EvaluationProblem>(p);
    }
}

Problem createRandomProblem(ProblemType type, const std::string& difficulty) {
    Problem p;
    p.difficulty = difficulty;
    p.type = type;
    
    switch (type) {
        case ProblemType::Evaluation: {
            auto poly = PolynomialFactory<double>::createRandom(2);
            p.title = "Polynomial Evaluation";
            p.description = "Evaluate the polynomial at x=2";
            p.polyCoeffs = PolynomialFactory<double>::coefficientsToString(poly.coeffs());
            break;
        }
        case ProblemType::RootFinding: {
            auto poly = PolynomialFactory<double>::createRandom(3);
            p.title = "Find Polynomial Roots";
            p.description = "Find one real root of the polynomial";
            p.polyCoeffs = PolynomialFactory<double>::coefficientsToString(poly.coeffs());
            break;
        }
        case ProblemType::Simplification: {
            p.title = "Polynomial Simplification";
            p.description = "Simplify the polynomial expression";
            p.polyCoeffs = "1,2,1"; // (x+1)^2
            break;
        }
        default:
            break;
    }
    
    return p;
}