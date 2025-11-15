#include <iostream>
#include <memory>
#include "DbManager.h"
#include "Problems.h"

int main() {
    DbManager db;

    try {
        db.connect("PolyRankDSN", "root", "password");
    } catch (std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    std::string username, pass;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, pass);

    User u;
    try {
        u = db.getUserByUsername(username);
        if (u.password != pass) {
            std::cout << "Invalid password.\n";
            return 0;
        }
    } catch (...) {
        std::cout << "User not found. Creating...\n";
        u = db.createUser(username, pass);
    }

    auto problems = db.getProblems();

    std::cout << "\nProblems:\n";
    for (const auto& p : problems)
        std::cout << p.id << ". " << p.title << "\n";

    std::cout << "Select problem ID: ";
    int pid;
    std::cin >> pid;

    Problem chosen;
    for (auto& p : problems)
        if (p.id == pid)
            chosen = p;

    auto prob = createProblemInstance(chosen, db);

    std::cin.ignore();
    std::cout << prob->getPrompt();
    std::string ans;
    std::getline(std::cin, ans);

    double score;
    bool ok = prob->checkAnswer(ans, score);

    Submission s;
    s.userId = u.id;
    s.problemId = pid;
    s.userAnswer = ans;
    s.isCorrect = ok;
    s.score = score;

    db.insertSubmission(s);

    std::cout << (ok ? "Correct!" : "Wrong.") << " Score: " << score;

    return 0;
}
