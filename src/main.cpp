#include <iostream>
#include <memory>
#include "DbManager.h"
#include "Problems.h"

int main() {
    DbManager db;

    try {
        db.connect("PolyRankDSN", "root", "P@2005Sharma");
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