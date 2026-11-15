#pragma once
#include <array>
#include <algorithm>
#include <complex>
#include <cmath>

namespace EQPInteractionMath {
using Complex=std::complex<double>;
using Roots=std::array<Complex,5>;

inline bool laguerre(const std::array<Complex,6>& a,int degree,Complex& x) noexcept {
    constexpr double fractions[]{.5,.25,.75,.13,.38,.62,.88,1.};
    for(int iteration=1;iteration<=100;++iteration) {
        Complex b=a[size_t(degree)],d=0,f=0;
        double error=std::abs(b),radius=std::abs(x);
        for(int j=degree-1;j>=0;--j){f=x*f+d;d=x*d+b;b=x*b+a[size_t(j)];error=std::abs(b)+radius*error;}
        if(std::abs(b)<=2e-14*error)return true;
        const Complex g=d/b,h=g*g-2.*f/b;
        const Complex square=std::sqrt(double(degree-1)*(double(degree)*h-g*g));
        const Complex plus=g+square,minus=g-square;
        const Complex denominator=std::abs(plus)>std::abs(minus)?plus:minus;
        const Complex step=std::abs(denominator)>1e-30?double(degree)/denominator:std::polar(1+radius,double(iteration));
        const Complex next=x-step;
        if(std::abs(step)<1e-13*(1+std::abs(next))){x=next;return true;}
        x=iteration%10==0?x-fractions[(iteration/10-1)%8]*step:next;
        if(!std::isfinite(x.real())||!std::isfinite(x.imag()))return false;
    }
    return false;
}

inline bool roots(const double* coefficients,Roots& result) noexcept {
    double scale=0;for(int i=0;i<6;++i)scale=std::max(scale,std::abs(coefficients[i]));
    if(scale==0||coefficients[5]==0)return false;
    std::array<Complex,6> original{},working{};
    for(int i=0;i<6;++i)original[size_t(i)]=coefficients[i]/scale;
    working=original;
    for(int degree=5;degree>=1;--degree) {
        Complex root=0;
        if(!laguerre(working,degree,root))return false;
        if(std::abs(root.imag())<1e-10*(1+std::abs(root.real())))root={root.real(),0};
        result[size_t(degree-1)]=root;
        Complex b=working[size_t(degree)];
        for(int j=degree-1;j>=0;--j){const Complex c=working[size_t(j)];working[size_t(j)]=b;b=root*b+c;}
    }
    for(auto& root:result)if(!laguerre(original,5,root))return false;
    return true;
}

inline bool group(const Roots& values,double (&sections)[3][3]) noexcept {
    std::array<bool,5> used{};std::array<double,5> real{};int realCount=0,count=0;
    for(int i=0;i<5;++i) {
        if(used[size_t(i)])continue;
        Complex value=values[size_t(i)];
        if(std::abs(value.imag())<2e-7*(1+std::abs(value.real()))){real[size_t(realCount++)]=value.real();used[size_t(i)]=true;continue;}
        int partner=-1;double nearest=1e100;
        for(int j=i+1;j<5;++j)if(!used[size_t(j)]){double distance=std::abs(values[size_t(j)]-std::conj(value));if(distance<nearest){nearest=distance;partner=j;}}
        if(partner<0||nearest>1e-4*(1+std::abs(value))||count>=3)return false;
        value=(value+std::conj(values[size_t(partner)]))*.5;
        sections[count][0]=1;sections[count][1]=-2*value.real();sections[count][2]=std::norm(value);++count;
        used[size_t(i)]=used[size_t(partner)]=true;
    }
    std::sort(real.begin(),real.begin()+realCount);
    for(int i=0;i<realCount;i+=2) {
        if(count>=3)return false;
        sections[count][0]=1;sections[count][1]=-real[size_t(i)];sections[count][2]=0;
        if(i+1<realCount){sections[count][1]-=real[size_t(i+1)];sections[count][2]=real[size_t(i)]*real[size_t(i+1)];}
        ++count;
    }
    return count==3;
}

// Factor the interpolated continuous-time transfer before forming the audio biquads.
// A failed or unstable design is rejected, so the caller can retain a stable fallback.
inline bool toSections(const double* packed,double sampleRate,double (&sections)[3][5]) noexcept {
    Roots zeros{},poles{};
    if(!roots(packed,zeros)||!roots(packed+6,poles))return false;
    constexpr double omega=6283.1853071795864769;
    const double k=2*sampleRate;
    Complex numeratorDC=1,denominatorDC=1;
    for(auto& pole:poles){if(pole.real()>=0)return false;pole=(k+omega*pole)/(k-omega*pole);if(std::abs(pole)>=1)return false;denominatorDC*=1.-pole;}
    for(auto& zero:zeros){zero=(k+omega*zero)/(k-omega*zero);numeratorDC*=1.-zero;}
    if(std::abs(numeratorDC)<1e-30||packed[6]==0)return false;
    const Complex gain=(packed[0]/packed[6])*denominatorDC/numeratorDC;
    if(!std::isfinite(gain.real())||std::abs(gain.imag())>1e-5*(1+std::abs(gain.real())))return false;
    double numerator[3][3]{},denominator[3][3]{};
    if(!group(zeros,numerator)||!group(poles,denominator))return false;
    for(int i=0;i<3;++i){double g=i==0?gain.real():1.;sections[i][0]=g*numerator[i][0];sections[i][1]=g*numerator[i][1];sections[i][2]=g*numerator[i][2];sections[i][3]=denominator[i][1];sections[i][4]=denominator[i][2];}
    return true;
}
}
