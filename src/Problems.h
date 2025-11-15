#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Models.h"
#include "Polynomial.h"

class DbManager; // forward declaration

class PolynomialProblem {
public:
    explicit PolynomialProblem(const Problem& p) : problem_(p) {}
    virtual ~PolynomialProblem() {}

    virtual std::string getPrompt() const = 0;
    virtual bool checkAnswer(const std::string& userAnswer, double& score) = 0;

protected:
    Problem problem_;
};

class EvaluationProblem : public PolynomialProblem {
public:
    explicit EvaluationProblem(const Problem& p);

    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;

private:
    Polynomial<double> poly_;
    double x_;
    double expected_;
};

class RootFindingProblem : public PolynomialProblem {
public:
    RootFindingProblem(const Problem& p, DbManager& db);

    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;

private:
    Polynomial<double> poly_;
    std::vector<double> roots_;
};

class SimplificationProblem : public PolynomialProblem {
public:
    explicit SimplificationProblem(const Problem& p);

    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;

private:
    std::string expected_;
};

std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p, DbManager& db);
