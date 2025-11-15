#pragma once
#include <memory>
#include <string>
#include "Models.h"
#include "Polynomial.h"
#include "Exceptions.h"

class PolynomialProblem {
public:
    explicit PolynomialProblem(const Problem& p) : problem_(p) {}
    virtual ~PolynomialProblem() = default;

    const Problem& getProblem() const { return problem_; }

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
    double x_ = 0.0;
    double expected_ = 0.0;
};

class RootFindingProblem : public PolynomialProblem {
public:
    explicit RootFindingProblem(const Problem& p);

    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;

private:
    double expectedRoot_ = 0.0;
};

class SimplificationProblem : public PolynomialProblem {
public:
    explicit SimplificationProblem(const Problem& p);

    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;

private:
    std::string expectedCanonical_;
};

std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p);
