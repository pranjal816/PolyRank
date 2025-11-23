#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Models.h"
#include "Polynomial.h"
#include "PolynomialSolver.h"
#include "PolynomialFactory.h"

class DbManager;

class PolynomialProblem {
public:
    explicit PolynomialProblem(const Problem& p) : problem_(p) {}
    virtual ~PolynomialProblem() = default;
    
    virtual std::string getPrompt() const = 0;
    virtual bool checkAnswer(const std::string& userAnswer, double& score) = 0;
    virtual std::string getSolution() const = 0;
    virtual std::string getCorrectAnswer() const = 0;
    
    const Problem& getProblem() const { return problem_; }

protected:
    Problem problem_;
};

class EvaluationProblem : public PolynomialProblem {
public:
    explicit EvaluationProblem(const Problem& p);
    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;
    std::string getSolution() const override;
    std::string getCorrectAnswer() const override;

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
    std::string getSolution() const override;
    std::string getCorrectAnswer() const override;

private:
    Polynomial<double> poly_;
    std::vector<double> roots_;
};

class SimplificationProblem : public PolynomialProblem {
public:
    explicit SimplificationProblem(const Problem& p);
    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;
    std::string getSolution() const override;
    std::string getCorrectAnswer() const override;

private:
    std::string expected_;
    std::string polynomial_;
};

class CustomSolutionProblem : public PolynomialProblem {
public:
    CustomSolutionProblem(const Problem& p, DbManager& db);
    std::string getPrompt() const override;
    bool checkAnswer(const std::string& userAnswer, double& score) override;
    std::string getSolution() const override;
    std::string getCorrectAnswer() const override;

private:
    Polynomial<double> poly_;
    std::vector<double> roots_;
    std::string solution_;
};

std::unique_ptr<PolynomialProblem> createProblemInstance(const Problem& p, DbManager& db);
Problem createRandomProblem(ProblemType type, const std::string& difficulty = "Medium");