#include <iostream>
#include <memory>
#include "DbManager.h"
#include "TerminalUI.h"

int main() {
    std::cout << "Starting PolyRank Terminal System..." << std::endl;
    
    DbManager db;
    
    try {
        // Connect to database using direct connection (no DSN)
        std::cout << "Attempting to connect to database..." << std::endl;
        db.connect("", "root", "P@2005Sharma");
        std::cout << "Database connected successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Database connection failed: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting steps:" << std::endl;
        std::cerr << "1. Make sure MySQL is running" << std::endl;
        std::cerr << "2. Download and install MySQL ODBC Driver from:" << std::endl;
        std::cerr << "   https://dev.mysql.com/downloads/connector/odbc/" << std::endl;
        std::cerr << "3. Verify your MySQL credentials" << std::endl;
        std::cerr << "4. Make sure PolyRank database exists" << std::endl;
        return 1;
    }
    
    try {
        TerminalUI ui(db);
        ui.run();
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "PolyRank system shutdown." << std::endl;
    return 0;
}