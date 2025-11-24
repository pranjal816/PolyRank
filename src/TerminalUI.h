#pragma once
#include "DbManager.h"
#include "Problems.h"
#include "PolynomialSolver.h"
#include "PolynomialFactory.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>

class TerminalUI {
public:
    TerminalUI(DbManager& db);
    void run();

private:
    DbManager& db_;
    User currentUser_;
    
    void showMainMenu();
    void handleLogin();
    void handleRegister();
    void showUserDashboard();
    void solveExistingProblems();
    void solveRandomProblem();
    void solveCustomPolynomial();
    void viewLeaderboard();
    void viewUserProfile();
    void viewSolutionHistory();
    
    void displayProblem(std::unique_ptr<PolynomialProblem> problem);
    void handleProblemSolution(std::unique_ptr<PolynomialProblem> problem);
    void displayPolynomialSolution(const Polynomial<double>& poly);
    
    void printHeader(const std::string& title);
    void printSuccess(const std::string& message);
    void printError(const std::string& message);
    void printInfo(const std::string& message);
    void clearScreen();
    
    std::string getInput(const std::string& prompt);
    int getIntInput(const std::string& prompt);
    double getDoubleInput(const std::string& prompt);
    void waitForEnter();
    
};