#pragma once
#include <vector>
#include <sstream>
#include <cmath>
#include <string>
#include <functional>
#include "Exceptions.h"

template<typename T>
class Polynomial {
public:
    Polynomial() = default;
    
    explicit Polynomial(const std::vector<T>& coeffs) : coeffs_(coeffs) {
        if (coeffs.empty()) {
            throw InvalidPolynomialException("Coefficients cannot be empty");
        }
    }
    
    T evaluate(T x) const {
        if (coeffs_.empty()) return 0;
        
        T result = 0;
        T power = 1;
        for (T c : coeffs_) {
            result += c * power;
            power *= x;
        }
        return result;
    }
    
    Polynomial<T> derivative() const {
        if (coeffs_.size() <= 1) {
            return Polynomial<T>({0});
        }
        
        std::vector<T> derivCoeffs;
        for (size_t i = 1; i < coeffs_.size(); ++i) {
            derivCoeffs.push_back(coeffs_[i] * static_cast<T>(i));
        }
        return Polynomial<T>(derivCoeffs);
    }
    
    int degree() const {
        return static_cast<int>(coeffs_.size()) - 1;
    }
    
    std::string toString() const {
        if (coeffs_.empty()) return "0";
        
        std::ostringstream oss;
        bool firstTerm = true;
        
        for (int i = static_cast<int>(coeffs_.size()) - 1; i >= 0; --i) {
            T coeff = coeffs_[i];
            if (coeff == 0) continue;
            
            if (!firstTerm) {
                oss << (coeff > 0 ? " + " : " - ");
            } else if (coeff < 0) {
                oss << "-";
            }
            
            T absCoeff = std::abs(coeff);
            
            if (i == 0) {
                oss << absCoeff;
            } else if (i == 1) {
                if (absCoeff != 1) oss << absCoeff;
                oss << "x";
            } else {
                if (absCoeff != 1) oss << absCoeff;
                oss << "x^" << i;
            }
            
            firstTerm = false;
        }
        
        return oss.str();
    }
    
    const std::vector<T>& coeffs() const {
        return coeffs_;
    }
    
    static Polynomial<T> parse(const std::string& str) {
        std::vector<T> coeffs;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, ',')) {
            if (!token.empty()) {
                try {
                    coeffs.push_back(static_cast<T>(std::stod(token)));
                } catch (const std::exception&) {
                    throw InvalidPolynomialException("Invalid coefficient: " + token);
                }
            }
        }
        
        if (coeffs.empty()) {
            throw InvalidPolynomialException("No coefficients found");
        }
        
        return Polynomial<T>(coeffs);
    }

private:
    std::vector<T> coeffs_;
};