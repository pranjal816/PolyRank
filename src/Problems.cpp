#include "Problems.h"
#include "PolynomialSolver.h"
#include "DbManager.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// ---------------- Evaluation ----------------

EvaluationProblem::EvaluationProblem(const Problem& p)
    : PolynomialProblem(p)
{
    poly_ = Polynomial<double>({ -4, 0, 1 });
    x_ = 2.0;
    expected_ = poly_.evaluate(x_);
}

std::string EvaluationProblem::getPrompt() const {
    std::ostringstream oss;
    oss << "Problem: " << problem_.title << "\n"
        << problem_.description << "\n"
        << "Compute p(" << x_ << "): ";
    return oss.str();
}

bool EvaluationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    double ans = std::stod(userAnswer);
    double diff = std::fabs(ans - expected_);
    if (diff < 1e-3) {
        score = 10;
        return true;
    }
    score = 0;
    return false;
}


// ---------------- Root Finding ----------------

static Polynomial<double> parsePoly(const std::string& s) {
    std::vector<double> coeffs;
    std::stringstream ss(s);
    std::string tok;

    while (std::getline(ss, tok, ',')) {
        if (!tok.empty())
            coeffs.push_back(std::stod(tok));
    }
    return Polynomial<double>(coeffs);
}

RootFindingProblem::RootFindingProblem(const Problem& p, DbManager& db)
    : PolynomialProblem(p)
{
    poly_ = parsePoly(p.polyCoeffs);

    roots_ = db.getCachedRoots(p.id);

    if (roots_.empty()) {
        roots_ = PolynomialSolver::solveNewton(poly_);
        db.cacheRoots(p.id, roots_);
    }
}

std::string RootFindingProblem::getPrompt() const {
    return "Enter a real root of the polynomial: ";
}

bool RootFindingProblem::checkAnswer(const std::string& userAnswer, double& score) {
    double ans = std::stod(userAnswer);

    for (double r : roots_) {
        if (std::fabs(ans - r) < 1e-3) {
            score = 10;
            return true;
        }
    }

    score = 0;
    return false;
}


// ---------------- Simplification ----------------

SimplificationProblem::SimplificationProblem(const Problem& p)
    : PolynomialProblem(p)
{
    expected_ = "x^2+2x+1";
}

std::string SimplificationProblem::getPrompt() const {
    return "Enter simplified form (no spaces), e.g. x^2+2x+1: ";
}

bool SimplificationProblem::checkAnswer(const std::string& userAnswer, double& score) {
    std::string user = userAnswer;
    user.erase(remove_if(user.begin(), user.end(), ::isspace), user.end());
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


// ---------------- FACTORY ----------------

std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p, DbManager& db)
{
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
