#ifndef PRIVATE_APPLICATIONS_PACKETCACHE_WIGGINS_HPP
#define PRIVATE_APPLICATIONS_PACKETCACHE_WIGGINS_HPP
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <vector>
#ifndef NDEBUG
#include <cassert>
#endif
#include "private/isEmpty.hpp"
#include "umps/messageFormats/dataPacket.hpp"
namespace
{
/// @brief Utility routine for computing term in Wiggins interpolation
///        spline coefficients.
template<typename T>
[[maybe_unused]]
void computeWiWiMi(const T mi, T *wi, T *wimi)
{
    constexpr T huge = std::numeric_limits<T>::max();
    constexpr T eps = std::numeric_limits<T>::epsilon();
    // The goal is to compute w_i and w_i m_i = 1/max(|m_i|, epsilon)*m_i
    // The trick is that to let w_i m_i safely go to 0 and let the
    // denominator become really big when the slope is 0.
    *wi = huge;
    *wimi = 0;
    auto ami = std::abs(mi);
    if (ami > eps)
    {   
        *wi = 1/ami;
        // "If m_i and m_{i+1} have opposite signs then s_i is zero".
        // Realize, the copysign in the ensuing numerator will ensure
        // operations like 1 - 1, 1 + 1, -1 + 1, or -1 - 1.
        *wimi = std::copysign(1, mi);
    }   
}
/// @brief Implements first equation in Wiggins' Interpolation fo Digitized
///        Curves pg. 2077
template<typename U, typename T>
[[maybe_unused]] [[nodiscard]]
std::vector<double> computeNonUniformSlopes(const int n,
                                            const U *__restrict__ x,
                                            const T *__restrict__ y)
{
    constexpr double one = 1;
    // Handle the initial conditions
    std::vector<double> slopes(n);
    slopes.at(0) = static_cast<double> (y[1] - y[0])
                  /static_cast<double> (x[1] - x[0]);
    double *__restrict__ slopesPtr = slopes.data();
    for (int i = 1; i < n - 1; ++i)
    {
        auto dx  = static_cast<double> (x[i] - x[i-1]);
        auto dx1 = static_cast<double> (x[i+1] - x[i]);
        auto mi  = static_cast<double> (y[i] - y[i-1])/dx;
        auto mi1 = static_cast<double> (y[i+1] - y[i])/dx1;
        // w_i and w_i m_i = 1/max(|m_i|, epsilon)*m_i
        double wi, wimi;
        computeWiWiMi(mi, &wi, &wimi);
        // w_{i+1} and w_{i+1} m_{i+1} = 1/(max(|m_{i+1}|, epsilon)*m_{i+1}
        double wi1, wi1mi1;
        computeWiWiMi(mi1, &wi1, &wi1mi1);
        // s_i = (w_i*m_i + w_{i+1}*m_{i+1})/(w_{i} + w_{i+1})
        slopesPtr[i] = (wimi + wi1mi1)/(wi + wi1);
    }
    // Handle the final conditions
    slopes.at(n-1) = static_cast<double> (y[n-1] - y[n-2])
                    /static_cast<double> (x[n-1] - x[n-2]);
    // Compute the spline coefficients: Eqn 4 from
    // Monotone Piecewise Cubic Interpolation - Fritsch and Carlson 1980
    std::vector<double> splineCoeffs(4*(n - 1));
    double *__restrict__ splineCoeffsPtr = splineCoeffs.data();
    for (int i = 0; i < n - 1; ++i)
    {
        auto di  = slopesPtr[i];
        auto di1 = slopesPtr[i+1];
        auto dx  = static_cast<double> (x[i+1] - x[i]);
        auto dxi = one/dx;
        auto dxi2 = dxi*dxi;
        auto delta = static_cast<double> (y[i+1] - y[i])*dxi;
        splineCoeffsPtr[4*i+0] = static_cast<double> (y[i]);
        splineCoeffsPtr[4*i+1] = di;
        splineCoeffsPtr[4*i+2] = (-2*di - di1 + 3*delta)*dxi;
        splineCoeffsPtr[4*i+3] = (di + di1 - 2*delta)*dxi2;
    }
    return splineCoeffs;
}
/// @brief Locates the appropriate bin for spline evaluation.
/// @result The bin such that \f$ xi[bin] \le x < xi[bin+1] \f$.
template<typename T>
[[maybe_unused]] [[nodiscard]]
int locate(const T x,
           const int n,
           const T *__restrict__ xi,
           const int binHint)
{
    // First try searching the previous bin or next bin
    bool findBin = true;
    int bin = 0;
    if (binHint >= 0)
    {
        auto b = binHint;
        if (b >= 0 && b < n - 1)
        {
            if (x >= xi[b] && x < xi[b + 1])
            {
                bin = b;
                findBin = false;
            }
            else
            {
                if (b < n - 2)
                {
                    if (x >= xi[b + 1] && x < xi[b + 2])
                    {
                        bin = b + 1;
                        findBin = false;
                    }
                }
            }
        }
    }
    // Hunt for it smartly
    if (findBin)
    {
        if (x <= xi[0])
        {
            bin = 0;
        }
        else if (x >= xi[n - 1])
        {
            bin = n - 2;
        }
        else
        {
            auto low = std::lower_bound(xi, xi + n, x);
            bin = static_cast<int> (std::distance(xi,  low));
            if (bin < 0){bin = 0;}
            if (bin >= n){bin = n - 2;}
            // Check neighbors since lower bound is a nightmare
            if (x < xi[bin] && bin > 0)
            {
                bin = bin - 1;
            }
            if (x >= xi[bin + 1])
            {
                bin = bin + 1;
            }
        }
    }
    // Force it
    if (x < xi[bin] || x >= xi[bin + 1])
    {
        for (int i = 0; i < n - 1; ++i)
        {
            if (x >= xi[i] && x < xi[i+1])
            {
                bin = i;
                break;
            }
        }
    }
#ifndef NDEBUG
    assert(bin < n - 1);
    assert(x >= xi[bin] && x < xi[bin + 1]);
#endif
    return bin;
}
/*
/// @brief Evaluates the fourth-order spline
template<typename T>
[[nodiscard]] [[maybe_unused]]
double evaluate(T x,
                const int n,
                const T *__restrict__ xi,
                const double *__restrict__ splineCoeffs,
                int &binHint)
{
#ifndef NDEBUG
     assert(n > 0);
     assert(xi != nullptr);
     assert(splineCoeffs != nullptr);
#endif
    // Handle edges now
    if (x < xi[0])
    {
        binHint = 0;
        return xi[0];
    }
    if (x > xi[n-1])
    {
        binHint = n - 2;
        return xi[n-1];
    }
    // Find right bin
    int bin = locate(x, n, xi, binHint);
    // Evaluate spline, y = a + bx + cx^2 + dx^3, with Horner's method.
    double dx = (x - xi[bin]);
    double y = splineCoeffs[4*bin]
             + dx*(splineCoeffs[4*bin + 1]
                + dx*(splineCoeffs[4*bin + 2] + splineCoeffs[4*bin+3]*dx));
    //double dx2 = dx*dx;
    //double dx3 = dx2*dx;
    //double y = splineCoeffs[4*bin]
    //         + splineCoeffs[4*bin + 1]*dx
    //         + splineCoeffs[4*bin + 2]*dx2
    //         + splineCoeffs[4*bin + 3]*dx3;
    // Save where we were for next time
    binHint = bin;
    return y;
}
*/
/// @brief Evaluates the fourth-order spline.
/// @param[out] yv  The interpolated values at x.
/// @param[in] n    The number of points at which to interpolate.
/// @param[in] x    The n values at which to interpolate.  This is an array
///                 whose dimension is [n].
/// @param[in] nxi  The number of abscissas.
/// @param[in] xi   The abscissas.  This is an array whose dimension is [nxi].
/// @param[in] splineCoefficients  The spline coefficients.  This is an
///                                array whose dimension is [4*(nxi - 1)]
template<typename U, typename T>
[[maybe_unused]]
void evaluate(std::vector<T> *yv,
              const int n, const U *__restrict__ x,
              const int nxi, const U *__restrict__ xi,
              const double *__restrict__ splineCoeffs)
{
    yv->resize(n, 0);
    T *__restrict__ yPtr = yv->data();
    int binHint = -1;
    // Get all the bins
    std::vector<int> bins(n);
    for (int i = 0; i < n; ++i)
    {
        if (x[i] < xi[0])
        {
            binHint = 0;
            bins[i] =-1;
            yPtr[i] = splineCoeffs[0];
        }
        else if (x[i] > xi[nxi - 1]) 
        {
            binHint = n - 2;
            bins[i] =-2;
            yPtr[i] = splineCoeffs[4*(n - 2)]; // 4*(n-2) = 4*(n-1) - 4
        }
        else
        {
            bins[i] = locate(x[i], nxi, xi, binHint);
            binHint = bins[i];
        }
    }
    // Now evaluate all the splines with Horner's method
    for (int i = 0; i < n; ++i)
    {
        auto bin = bins[i];
        if (bin >= 0)
        {
            int indx = 4*bin;
            double dx = (x[i] - xi[bin]);
            double y = splineCoeffs[indx]
                     + dx*(splineCoeffs[indx + 1]
                     + dx*(splineCoeffs[indx + 2] + splineCoeffs[indx + 3]*dx));
            yPtr[i] = static_cast<T> (y);
        }
    }
}
/// @brief Performs the weighted average slopes interpolation.
template<typename T>
std::vector<T> weightedAverageSlopes(const std::vector<T> &times,
                                     const std::vector<T> &values,
                                     const double t0, const double t1,
                                     const double samplingRate = 100)
{
    if (samplingRate <= 0)
    {
        throw std::invalid_argument("Sampling rate must be positive");
    }
    if (times.size() != values.size())
    {
        throw std::invalid_argument("times.size() != values.size()");
    }
    if (times.size() < 2)
    {
        throw std::invalid_argument("At least two abscissas needed");
    }
    if (t1 <= t0)
    {
        throw std::invalid_argument("t1 must be greater than t1");
    }
    double dt = 1./samplingRate;
    auto nInterp = static_cast<int> (std::round( (t1 - t0)/dt )) + 1;
    while (t0 + (nInterp - 1)*dt >= t1 && nInterp > -1)
    {
        nInterp = nInterp - 1;
    }
    // Compute the interpolation times.  Basically this check means that
    // t1 - t0 < dt so we simply interpolate at t0 and call it a day
    std::vector<T> timesToEvaluate(std::max(nInterp, 1), t0);
    T *__restrict__ tPtr = timesToEvaluate.data();
    for (int i = 0; i < static_cast<int> (timesToEvaluate.size()); ++i)
    {
        tPtr[i] = static_cast<T> (t0 + i*dt);
    }
    // Try to squeeze one more in there
    if (std::abs(t1 - (timesToEvaluate.back() + dt)) <
        std::numeric_limits<T>::epsilon()*100)
    {
        timesToEvaluate.push_back(t1);
    }
    // Now interpolate
    auto splineCoefficients = computeNonUniformSlopes(times.size(),
                                                      times.data(),
                                                      values.data());
    std::vector<T> yHat;
    evaluate<T>(&yHat,
                timesToEvaluate.size(),
                timesToEvaluate.data(),
                times.size(), times.data(),
                splineCoefficients.data());
    return yHat;
}
}
#endif
