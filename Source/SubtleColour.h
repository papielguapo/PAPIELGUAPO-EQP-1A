#pragma once
#include <cmath>

// Symmetric parallel soft saturation. Only the nonlinear residual uses ADAA;
// the original EQ path keeps its phase and reported latency.
struct SubtleColour {
    double previous = 0;
    static double residual(double x) noexcept { return -.25*x*x*x/(1+.25*x*x); }
    static double primitive(double x) noexcept {
        const double q=.25*x*x;
        // Avoid cancellation near silence.
        if(q<.001) return x*x*q*(-.25+q*(1./6-q/8+q*q/10));
        return 2*std::log1p(q)-.5*x*x;
    }
    double tick(double x) noexcept {
        const double delta=x-previous;
        const double r=std::abs(delta)<1e-5*(1+std::abs(x)+std::abs(previous))
            ? residual(.5*(x+previous)) : (primitive(x)-primitive(previous))/delta;
        previous=x;
        return x+.2*r;
    }
    void reset() noexcept { previous=0; }
};
