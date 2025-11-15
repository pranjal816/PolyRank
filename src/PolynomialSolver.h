#pragma once
#include <vector>
#include "Polynomial.h"

class PolynomialSolver {
public:
    static std::vector<double> solveNewton(const Polynomial<double>& poly,
                                           double tolerance = 1e-6,
                                           int maxIterations = 1000);
};
