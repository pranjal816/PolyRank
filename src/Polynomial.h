#pragma once
#include <vector>
#include <sstream>
#include <cmath>
#include <string>

template<typename T>
class Polynomial {
public:
    Polynomial() = default;
    explicit Polynomial(const std::vector<T>& coeffs) : coeffs_(coeffs) {}

    T evaluate(T x) const {
        T result = 0;
        T power = 1;
        for (T c : coeffs_) {
            result += c * power;
            power *= x;
        }
        return result;
    }

    int degree() const {
        return coeffs_.size() - 1;
    }

    const std::vector<T>& coeffs() const {
        return coeffs_;
    }

private:
    std::vector<T> coeffs_;
};
