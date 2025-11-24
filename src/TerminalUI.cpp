#include "TerminalUI.h"
#include <limits>
#include <algorithm>
#include <map>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

TerminalUI::TerminalUI(DbManager& db) : db_(db) {}

void TerminalUI::run() {
    clearScreen();
    printHeader("=== PolyRank - Polynomial Learning System ===");
    
    while (true) {
        std::cout << "\n1. Login\n";
        std::cout << "2. Register\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose an option: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1:
                handleLogin();
                break;
            case 2:
                handleRegister();
                break;
            case 3:
                std::cout << "Thank you for using PolyRank!\n";
                return;
            default:
                printError("Invalid choice. Please try again.");
        }
    }
}

void TerminalUI::handleLogin() {
    clearScreen();
    printHeader("Login");
    
    std::string username = getInput("Enter username: ");
    std::string password = getInput("Enter password: ");
    
    try {
        currentUser_ = db_.getUserByUsername(username);
        if (currentUser_.password == password) {
            printSuccess("Login successful! Welcome back, " + username + "!");
            waitForEnter();
            showUserDashboard();
        } else {
            printError("Invalid password!");
            waitForEnter();
        }
    } catch (const std::exception& e) {
        printError("User not found!");
        waitForEnter();
    }
}

void TerminalUI::handleRegister() {
    clearScreen();
    printHeader("Register New Account");
    
    std::string username = getInput("Enter username: ");
    std::string password = getInput("Enter password: ");
    
    try {
        currentUser_ = db_.createUser(username, password);
        printSuccess("Registration successful! Welcome to PolyRank, " + username + "!");
        waitForEnter();
        showUserDashboard();
    } catch (const std::exception& e) {
        printError("Registration failed: " + std::string(e.what()));
        waitForEnter();
    }
}

void TerminalUI::showUserDashboard() {
    while (true) {
        clearScreen();
        printHeader("Dashboard - Welcome, " + currentUser_.username + "!");
        
        // Display user stats
        auto submissions = db_.getUserSubmissions(currentUser_.id);
        int problemsSolved = 0;
        double totalScore = 0.0;
        
        for (const auto& sub : submissions) {
            if (sub.isCorrect) {
                problemsSolved++;
                totalScore += sub.score;
            }
        }
        
        std::cout << "📊 Your Statistics:\n";
        std::cout << "   Problems Solved: " << problemsSolved << "\n";
        std::cout << "   Total Score: " << totalScore << "\n";
        std::cout << "   Accuracy: " << (submissions.empty() ? 0 : (problemsSolved * 100.0 / submissions.size())) << "%\n\n";
        
        std::cout << "Main Menu:\n";
        std::cout << "1. Solve Existing Problems\n";
        std::cout << "2. Solve Random Problem\n";
        std::cout << "3. Custom Polynomial Solver\n";
        std::cout << "4. View Leaderboard\n";
        std::cout << "5. View My Profile\n";
        std::cout << "6. View Solution History\n";
        std::cout << "7. Logout\n";
        std::cout << "Choose an option: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1:
                solveExistingProblems();
                break;
            case 2:
                solveRandomProblem();
                break;
            case 3:
                solveCustomPolynomial();
                break;
            case 4:
                viewLeaderboard();
                break;
            case 5:
                viewUserProfile();
                break;
            case 6:
                viewSolutionHistory();
                break;
            case 7:
                currentUser_ = User(); // Clear current user
                return;
            default:
                printError("Invalid choice!");
                waitForEnter();
        }
    }
}

void TerminalUI::solveExistingProblems() {
    clearScreen();
    printHeader("Available Problems");
    
    try {
        auto problems = db_.getProblems();
        
        if (problems.empty()) {
            printInfo("No problems available at the moment.");
            waitForEnter();
            return;
        }
        
        std::cout << "Available Problems:\n";
        std::cout << std::string(60, '-') << "\n";
        for (const auto& problem : problems) {
            std::cout << "ID: " << problem.id << " | " << problem.title << "\n";
            std::cout << "   Difficulty: " << problem.difficulty;
            std::cout << " | Type: " << problemTypeToString(problem.type) << "\n";
            std::cout << "   " << problem.description << "\n\n";
        }
        
        int problemId = getIntInput("Enter Problem ID to solve (0 to go back): ");
        if (problemId == 0) return;
        
        Problem selectedProblem;
        for (const auto& problem : problems) {
            if (problem.id == problemId) {
                selectedProblem = problem;
                break;
            }
        }
        
        if (selectedProblem.id == 0) {
            printError("Problem not found!");
            waitForEnter();
            return;
        }
        
        auto problemInstance = createProblemInstance(selectedProblem, db_);
        handleProblemSolution(std::move(problemInstance));
        
    } catch (const std::exception& e) {
        printError("Error loading problems: " + std::string(e.what()));
        waitForEnter();
    }
}

void TerminalUI::solveRandomProblem() {
    clearScreen();
    printHeader("Random Problem Generator");
    
    std::cout << "Choose problem type:\n";
    std::cout << "1. Polynomial Evaluation\n";
    std::cout << "2. Root Finding\n";
    std::cout << "3. Polynomial Simplification\n";
    std::cout << "Choose type: ";
    
    int typeChoice;
    std::cin >> typeChoice;
    std::cin.ignore();
    
    ProblemType problemType;
    switch (typeChoice) {
        case 1:
            problemType = ProblemType::Evaluation;
            break;
        case 2:
            problemType = ProblemType::RootFinding;
            break;
        case 3:
            problemType = ProblemType::Simplification;
            break;
        default:
            printError("Invalid choice!");
            waitForEnter();
            return;
    }
    
    // Generate random problem
    Problem randomProblem = createRandomProblem(problemType);
    auto problemInstance = createProblemInstance(randomProblem, db_);
    
    std::cout << "\n🎯 Random Problem Generated!\n";
    handleProblemSolution(std::move(problemInstance));
}

void TerminalUI::solveCustomPolynomial() {
    clearScreen();
    printHeader("Custom Polynomial Solver");
    
    std::cout << "You can solve any polynomial equation of any degree!\n\n";
    std::cout << "Enter polynomial coefficients separated by commas (highest degree first).\n";
    std::cout << "Example: For x² - 3x + 2, enter: 1,-3,2\n";
    std::cout << "Example: For x³ - 2x + 1, enter: 1,0,-2,1\n\n";
    
    std::string coefficients = getInput("Enter coefficients: ");
    
    try {
        auto poly = Polynomial<double>::parse(coefficients);
        
        std::cout << "\n🧮 Polynomial: " << poly.toString() << "\n";
        std::cout << "📐 Degree: " << poly.degree() << "\n\n";
        
        // Get solving parameters
        double tolerance = getDoubleInput("Enter tolerance (default 0.000001): ");
        if (tolerance <= 0) tolerance = 1e-6;
        
        int maxIterations = getIntInput("Enter maximum iterations (default 1000): ");
        if (maxIterations <= 0) maxIterations = 1000;
        
        std::cout << "\n🔍 Solving polynomial using Newton-Raphson method...\n";
        
        auto roots = PolynomialSolver<double>::solveNewton(poly, tolerance, maxIterations);
        
        std::cout << "\n✅ Solution Results:\n";
        std::cout << std::string(40, '-') << "\n";
        
        if (roots.empty()) {
            std::cout << "No real roots found within the given tolerance.\n";
        } else {
            std::cout << "Real Roots Found: ";
            for (size_t i = 0; i < roots.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << std::fixed << std::setprecision(6) << roots[i];
            }
            std::cout << "\n";
            
            // Verify roots
            std::cout << "\n🔍 Verification:\n";
            for (double root : roots) {
                double verification = poly.evaluate(root);
                std::cout << "  p(" << std::setprecision(6) << root << ") = " 
                         << std::scientific << verification;
                if (std::fabs(verification) < tolerance * 10) {
                    std::cout << " ✓ (Valid root)\n";
                } else {
                    std::cout << " ⚠ (May not be accurate)\n";
                }
            }
        }
        
        // Save to custom solutions
        CustomSolutionRequest request;
        request.userId = currentUser_.id;
        request.polynomial = coefficients;
        
        std::ostringstream oss;
        oss << "Roots: ";
        for (size_t i = 0; i < roots.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << roots[i];
        }
        request.solution = oss.str();
        
        db_.insertCustomSolutionRequest(request);
        
        std::cout << "\n💾 Solution saved to your history.\n";
        
    } catch (const std::exception& e) {
        printError("Error solving polynomial: " + std::string(e.what()));
    }
    
    waitForEnter();
}

void TerminalUI::handleProblemSolution(std::unique_ptr<PolynomialProblem> problem) {
    clearScreen();
    printHeader("Solve Problem");
    
    std::cout << problem->getPrompt() << "\n\n";
    
    std::string userAnswer = getInput("Your answer: ");
    
    double score;
    bool isCorrect = problem->checkAnswer(userAnswer, score);
    
    // Create and save submission
    Submission submission;
    submission.userId = currentUser_.id;
    submission.problemId = problem->getProblem().id;
    submission.userAnswer = userAnswer;
    submission.isCorrect = isCorrect;
    submission.score = score;
    submission.correctAnswer = problem->getCorrectAnswer();
    
    db_.insertSubmission(submission);
    
    std::cout << "\n" << (isCorrect ? "✅ Correct!" : "❌ Wrong!") << "\n";
    std::cout << "📊 Score: " << score << "/10\n";
    
    if (!isCorrect) {
        std::cout << "💡 Correct answer: " << problem->getCorrectAnswer() << "\n";
    }
    
    std::cout << "\nShow detailed solution? (y/n): ";
    char showSolution;
    std::cin >> showSolution;
    std::cin.ignore();
    
    if (showSolution == 'y' || showSolution == 'Y') {
        std::cout << "\n📖 Detailed Solution:\n";
        std::cout << std::string(40, '-') << "\n";
        std::cout << problem->getSolution() << "\n";
    }
    
    waitForEnter();
}

void TerminalUI::viewLeaderboard() {
    clearScreen();
    printHeader("Leaderboard");
    
    try {
        auto leaderboard = db_.getLeaderboard();
        
        if (leaderboard.empty()) {
            printInfo("No leaderboard data available.");
            waitForEnter();
            return;
        }
        
        std::cout << "🏆 Top Polynomial Solvers:\n";
        std::cout << std::string(60, '-') << "\n";
        std::cout << std::setw(4) << "Rank" << std::setw(20) << "Username" 
                  << std::setw(15) << "Score" << std::setw(15) << "Solved\n";
        std::cout << std::string(60, '-') << "\n";
        
        int rank = 1;
        for (const auto& entry : leaderboard) {
            std::cout << std::setw(4) << rank++ 
                      << std::setw(20) << entry.first
                      << std::setw(15) << std::fixed << std::setprecision(1) << entry.second
                      << std::setw(15) << "?\n"; // Would need additional DB query for problems solved
        }
        
        // Show user's position
        std::cout << "\n👤 Your Position:\n";
        // This would require additional logic to find user's rank
        
    } catch (const std::exception& e) {
        printError("Error loading leaderboard: " + std::string(e.what()));
    }
    
    waitForEnter();
}

void TerminalUI::viewUserProfile() {
    clearScreen();
    printHeader("User Profile - " + currentUser_.username);
    
    try {
        auto submissions = db_.getUserSubmissions(currentUser_.id);
        
        int totalProblems = submissions.size();
        int correctProblems = 0;
        double totalScore = 0.0;
        
        for (const auto& sub : submissions) {
            if (sub.isCorrect) {
                correctProblems++;
                totalScore += sub.score;
            }
        }
        
        std::cout << "📈 Performance Statistics:\n";
        std::cout << std::string(40, '-') << "\n";
        std::cout << "Total Problems Attempted: " << totalProblems << "\n";
        std::cout << "Problems Solved Correctly: " << correctProblems << "\n";
        std::cout << "Total Score: " << totalScore << "\n";
        std::cout << "Accuracy: " << (totalProblems > 0 ? (correctProblems * 100.0 / totalProblems) : 0) << "%\n";
        std::cout << "Average Score: " << (totalProblems > 0 ? (totalScore / totalProblems) : 0) << "/10\n";
        
        std::cout << "\n🕐 Recent Activity (Last 5 submissions):\n";
        std::cout << std::string(60, '-') << "\n";
        
        int count = 0;
        for (const auto& sub : submissions) {
            if (count++ >= 5) break;
            std::cout << (sub.isCorrect ? "✅" : "❌") << " Problem " << sub.problemId 
                      << " | Score: " << sub.score << "/10 | Answer: " << sub.userAnswer << "\n";
        }
        
    } catch (const std::exception& e) {
        printError("Error loading profile: " + std::string(e.what()));
    }
    
    waitForEnter();
}

void TerminalUI::viewSolutionHistory() {
    clearScreen();
    printHeader("Solution History");
    
    try {
        auto submissions = db_.getUserSubmissions(currentUser_.id);
        auto customSolutions = db_.getCustomSolutionRequests(currentUser_.id);
        
        if (submissions.empty() && customSolutions.empty()) {
            printInfo("No solution history found.");
            waitForEnter();
            return;
        }
        
        if (!submissions.empty()) {
            std::cout << "📝 Problem Submissions:\n";
            std::cout << std::string(70, '-') << "\n";
            for (const auto& sub : submissions) {
                std::cout << "Problem " << sub.problemId << " | " 
                         << (sub.isCorrect ? "✅ Correct" : "❌ Incorrect")
                         << " | Score: " << sub.score << "/10\n";
                std::cout << "  Your answer: " << sub.userAnswer << "\n";
                if (!sub.isCorrect) {
                    std::cout << "  Correct answer: " << sub.correctAnswer << "\n";
                }
                std::cout << "  Submitted: " << sub.submittedAt << "\n\n";
            }
        }
        
        if (!customSolutions.empty()) {
            std::cout << "🔧 Custom Solutions:\n";
            std::cout << std::string(70, '-') << "\n";
            for (const auto& sol : customSolutions) {
                std::cout << "Polynomial: " << sol.polynomial << "\n";
                std::cout << "Solution: " << sol.solution << "\n";
                std::cout << "Requested: " << sol.requestedAt << "\n\n";
            }
        }
        
    } catch (const std::exception& e) {
        printError("Error loading solution history: " + std::string(e.what()));
    }
    
    waitForEnter();
}

// Utility functions
void TerminalUI::printHeader(const std::string& title) {
    std::cout << title << "\n";
    std::cout << std::string(title.length(), '=') << "\n\n";
}

void TerminalUI::printSuccess(const std::string& message) {
    std::cout << "✅ " << message << "\n";
}

void TerminalUI::printError(const std::string& message) {
    std::cout << "❌ " << message << "\n";
}

void TerminalUI::printInfo(const std::string& message) {
    std::cout << "💡 " << message << "\n";
}

void TerminalUI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

std::string TerminalUI::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int TerminalUI::getIntInput(const std::string& prompt) {
    std::cout << prompt;
    int value;
    std::cin >> value;
    std::cin.ignore();
    return value;
}

void TerminalUI::waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
}

double TerminalUI::getDoubleInput(const std::string& prompt) {
    std::cout << prompt;
    double value;
    std::cin >> value;
    std::cin.ignore();
    return value;
}

void TerminalUI::waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
}