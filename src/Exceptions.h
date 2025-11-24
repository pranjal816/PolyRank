#pragma once
#include <stdexcept>
#include <string>

class PolynomialException : public std::runtime_error {
public:
    explicit PolynomialException(const std::string& msg)
        : std::runtime_error("Polynomial Error: " + msg) {}
};

class InvalidPolynomialException : public PolynomialException {
public:
    explicit InvalidPolynomialException(const std::string& msg)
        : PolynomialException("Invalid Polynomial: " + msg) {}
};

class SolverException : public PolynomialException {
public:
    explicit SolverException(const std::string& msg)
        : PolynomialException("Solver Error: " + msg) {}
};

class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& msg)
        : std::runtime_error("DB Error: " + msg) {}
};