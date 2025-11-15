#pragma once
#include <vector>
#include <sstream>
#include <cmath>
#include <string>

template <typename T>
class Polynomial {
public:
    Polynomial() = default;

    explicit Polynomial(const std::vector<T>& coeffs)
        : coeffs_(coeffs) {}

    T evaluate(T x) const {
        T result = 0;
        T power = 1;
        for (T c : coeffs_) {
            result += c * power;
            power *= x;
        }
        return result;
    }

    std::string toString() const {
        std::ostringstream oss;
        bool first = true;
        for (int i = static_cast<int>(coeffs_.size()) - 1; i >= 0; --i) {
            T c = coeffs_[i];
            if (std::abs(static_cast<double>(c)) < 1e-12) continue;

            if (!first) {
                oss << (c >= 0 ? " + " : " - ");
            } else if (c < 0) {
                oss << "-";
            }

            T absC = c >= 0 ? c : -c;

            if (i == 0) {
                oss << absC;
            } else {
                if (absC != 1) oss << absC;
                oss << "x";
                if (i > 1) oss << "^" << i;
            }
            first = false;
        }
        if (first) oss << "0";
        return oss.str();
    }

    int degree() const {
        return coeffs_.empty() ? -1 : static_cast<int>(coeffs_.size()) - 1;
    }

    const std::vector<T>& coeffs() const {
        return coeffs_;
    }

private:
    // coeffs_[i] is coefficient for x^i
    std::vector<T> coeffs_;
};
