#pragma once
#include <vector>
#include "Polynomial.h"

class PolynomialSolver {
public:
    // Solve for all real roots using Newton–Raphson (with deflation)
    static std::vector<double> solveNewton(const Polynomial<double>& poly,
                                           double tolerance = 1e-6,
                                           int maxIterations = 1000);

private:
    static double derivative(const Polynomial<double>& p, double x);
    static double newtonSingleRoot(const Polynomial<double>& p, double x0,
                                   double tolerance, int maxIterations);
    static Polynomial<double> deflate(const Polynomial<double>& p, double root);
};
