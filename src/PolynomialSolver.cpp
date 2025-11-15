#include "PolynomialSolver.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

static double derivative(const Polynomial<double>& p, double x) {
    double h = 1e-6;
    return (p.evaluate(x + h) - p.evaluate(x - h)) / (2 * h);
}

static double newtonSingleRoot(const Polynomial<double>& p, double x0,
                               double tolerance, int maxIterations) {
    double x = x0;
    for (int i = 0; i < maxIterations; ++i) {
        double fx = p.evaluate(x);
        double dfx = derivative(p, x);

        if (std::fabs(dfx) < 1e-12) {
            x += 0.1;
            continue;
        }

        double xNext = x - fx / dfx;
        if (std::fabs(xNext - x) < tolerance)
            return xNext;

        x = xNext;
    }
    return x;
}

static Polynomial<double> deflatePoly(const Polynomial<double>& p, double root) {
    const auto& asc = p.coeffs();
    int n = asc.size() - 1;
    if (n <= 0) return Polynomial<double>({ 0.0 });

    std::vector<double> desc(n + 1);
    for (int i = 0; i <= n; ++i)
        desc[n - i] = asc[i];

    std::vector<double> qdesc(n);
    double b = desc[0];
    qdesc[0] = b;

    for (int i = 1; i <= n - 1; ++i) {
        b = desc[i] + root * b;
        qdesc[i] = b;
    }

    std::vector<double> qasc(n);
    for (int i = 0; i < n; ++i)
        qasc[n - 1 - i] = qdesc[i];

    return Polynomial<double>(qasc);
}

std::vector<double> PolynomialSolver::solveNewton(const Polynomial<double>& poly,
                                                  double tolerance,
                                                  int maxIterations) {
    Polynomial<double> p = poly;
    std::vector<double> roots;

    int deg = p.degree();
    if (deg <= 0) return roots;

    std::srand((unsigned)std::time(nullptr));

    for (int k = 0; k < deg; ++k) {
        double guess = (std::rand() % 200 - 100) / 10.0;

        double root = newtonSingleRoot(p, guess, tolerance, maxIterations);
        roots.push_back(root);

        p = deflatePoly(p, root);

        if (p.degree() <= 0)
            break;
    }
    return roots;
}
