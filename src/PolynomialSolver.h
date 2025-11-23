#pragma once
#include <vector>
#include <functional>
#include "Polynomial.h"
#include "Exceptions.h"

template<typename T>
class PolynomialSolver {
public:
    using FunctionType = std::function<T(T)>;
    
    static std::vector<T> solveNewton(const Polynomial<T>& poly,
                                    T tolerance = 1e-6,
                                    int maxIterations = 1000);
    
    static std::vector<T> solveNewtonWithFunction(FunctionType func,
                                                FunctionType deriv,
                                                T tolerance = 1e-6,
                                                int maxIterations = 1000);
    
    static T evaluateFunction(FunctionType func, T x);
    static T evaluateDerivative(FunctionType func, T x);

private:
    static T newtonSingleRoot(const Polynomial<T>& p, T x0,
                            T tolerance, int maxIterations);
    
    static T newtonSingleRootWithFunction(FunctionType func, FunctionType deriv,
                                        T x0, T tolerance, int maxIterations);
    
    static Polynomial<T> deflatePoly(const Polynomial<T>& p, T root);
};