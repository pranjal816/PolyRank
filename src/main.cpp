#include <iostream>
#include <memory>
#include <limits>
#include <algorithm>

#include "DbManager.h"
#include "Problems.h"

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    std::cout << "=== PolyRank - Polynomial Problem Solving and Ranking System ===\n";

    DbManager db;
    try {
        // Adjust DSN/credentials for your MySQL ODBC DSN
        // DSN must connect to database: polyrank_db
        db.connect("PolyRankDSN", "root", "P@2005Sharma");
    } catch (const DbException& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    std::string username, password;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    User currentUser;
    try {
        currentUser = db.getUserByUsername(username);
        if (currentUser.password != password) {
            std::cerr << "Invalid password.\n";
            return 1;
        }
        std::cout << "Welcome back, " << currentUser.username << "!\n";
    } catch (const DbException&) {
        std::cout << "User not found. Creating new user...\n";
        try {
            currentUser = db.createUser(username, password);
            std::cout << "User created. Welcome, " << currentUser.username << "!\n";
        } catch (const DbException& ex) {
            std::cerr << ex.what() << "\n";
            return 1;
        }
    }

    bool running = true;
    while (running) {
        std::cout << "\nMain Menu\n"
                  << "1. List and solve problems\n"
                  << "2. View leaderboard\n"
                  << "3. Exit\n"
                  << "Choose an option: ";
        int choice;
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid input.\n";
            continue;
        }
        clearInput();

        if (choice == 1) {
            std::vector<Problem> problems;
            try {
                problems = db.getProblems();
            } catch (const DbException& ex) {
                std::cerr << ex.what() << "\n";
                continue;
            }

            if (problems.empty()) {
                std::cout << "No problems available.\n";
                continue;
            }

            std::cout << "\nAvailable Problems:\n";
            for (const auto& p : problems) {
                std::cout << p.id << ". " << p.title
                          << " [" << p.difficulty << ", "
                          << problemTypeToString(p.type) << "]\n";
            }

            std::cout << "Enter problem ID to solve: ";
            int pid;
            if (!(std::cin >> pid)) {
                clearInput();
                std::cout << "Invalid input.\n";
                continue;
            }
            clearInput();

            auto it = std::find_if(problems.begin(), problems.end(),
                                   [pid](const Problem& p) { return p.id == pid; });
            if (it == problems.end()) {
                std::cout << "Problem not found.\n";
                continue;
            }

            std::unique_ptr<PolynomialProblem> prob = createProblemInstance(*it, db);
            std::cout << "\n" << prob->getPrompt();

            std::string answer;
            std::getline(std::cin, answer);

            bool correct = false;
            double score = 0.0;
            try {
                correct = prob->checkAnswer(answer, score);
            } catch (const InvalidPolynomialException& ex) {
                std::cout << ex.what() << "\n";
                continue;
            }

            std::cout << (correct ? "Correct! " : "Incorrect. ")
                      << "Score: " << score << "\n";

            Submission s;
            s.userId = currentUser.id;
            s.problemId = it->id;
            s.userAnswer = answer;
            s.isCorrect = correct;
            s.score = score;

            try {
                db.insertSubmission(s);
            } catch (const DbException& ex) {
                std::cerr << "Failed to save submission: " << ex.what() << "\n";
            }

        } else if (choice == 2) {
            try {
                auto lb = db.getLeaderboard();
                if (lb.empty()) {
                    std::cout << "No submissions yet.\n";
                } else {
                    std::cout << "\nLeaderboard (username : total score)\n";
                    for (const auto& entry : lb) {
                        std::cout << entry.first << " : " << entry.second << "\n";
                    }
                }
            } catch (const DbException& ex) {
                std::cerr << ex.what() << "\n";
            }

        } else if (choice == 3) {
            running = false;
        } else {
            std::cout << "Invalid option.\n";
        }
    }

    std::cout << "Goodbye!\n";
    return 0;
}
