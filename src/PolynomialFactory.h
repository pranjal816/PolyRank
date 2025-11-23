#pragma once
#include "Polynomial.h"
#include "Exceptions.h"
#include <random>
#include <string>
#include <sstream>

template<typename T>
class PolynomialFactory {
public:
    static Polynomial<T> createRandom(int degree, T minCoeff = -5, T maxCoeff = 5) {
        if (degree < 0) {
            throw InvalidPolynomialException("Degree must be non-negative");
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis(minCoeff, maxCoeff);
        
        std::vector<T> coeffs;
        for (int i = 0; i <= degree; ++i) {
            T coeff = dis(gen);
            if (i == degree && coeff == 0) {
                coeff = 1;
            }
            coeffs.push_back(coeff);
        }
        
        return Polynomial<T>(coeffs);
    }
    
    static std::string coefficientsToString(const std::vector<T>& coeffs) {
        std::ostringstream oss;
        for (size_t i = 0; i < coeffs.size(); ++i) {
            if (i > 0) oss << ",";
            oss << coeffs[i];
        }
        return oss.str();
    }
    
    static std::vector<T> stringToCoefficients(const std::string& str) {
        return Polynomial<T>::parse(str).coeffs();
    }
};