#pragma once
#include <stdexcept>
#include <string>

class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& msg)
        : std::runtime_error("DB Error: " + msg) {}
};

class InvalidPolynomialException : public std::runtime_error {
public:
    explicit InvalidPolynomialException(const std::string& msg)
        : std::runtime_error("Invalid Polynomial: " + msg) {}
};
