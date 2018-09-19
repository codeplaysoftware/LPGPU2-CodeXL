/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include<functional>
#include<cmath>
#include<iostream>



#define SIM_LO -1



#define SIM_PREC double



#define SIM_CTOR(C,T,F) C(T delta = 1, T lo = SIM_LO, T hi = 1, T phase = 0):SimCounter<T>(F,delta,lo,hi,phase) {}



template<typename T = SIM_PREC>
class SimCounter {
 std::function<T(T)> _fun;

 size_t _count = 0;

protected:
 T _delta;  // increase in 't' per step
 T _lo,_hi; // min and max values of the function
 T _phase;  // a 'shift' parameter to prevent counters of same period all lining up

public:
 SimCounter(std::function<T(T)> fun, T delta, T lo = SIM_LO, T hi = 1, T phase = 0):_fun(fun),_delta(delta),_lo(lo),_hi(hi),_phase(phase) {}

 SimCounter()                  = delete;
 SimCounter(SimCounter const&) = default;

 // The interface...
 T operator()() {T r = (*this)(_count); _count++; return r;}

 // Random access...
 T operator()(size_t count) {return _lo + (_hi - _lo) * (_fun(count * _delta + _phase) + 1) / 2;}

 // Reset the counter to zero...
 void reset() {_count=0;}

 // Update the step parameter based on signal frequency and sample period
 void updateDelta(T signalFreqHz, T samplePeriodNanos) {_delta = 2.e-9 * M_PI * signalFreqHz * samplePeriodNanos;}

 // Convenience function to see some values...
 void example(std::size_t num, std::string title) {
  std::cout << "\n" << title << "...\n";

  for(int i=0;i<=num;i++) std::cout << (*this)() << "\n";
 }
}; // SimCounter



// Convenience Counter Classes...

template<typename T = SIM_PREC>
class SinCounter: public SimCounter<T> {
public:
 SIM_CTOR(SinCounter,T,[](T t){return std::sin(t);})
}; // SinCounter



template<typename T = SIM_PREC>
class CosCounter: public SimCounter<T> {
public:
 SIM_CTOR(CosCounter,T,[](T t){return std::cos(t);})
}; // CosCounter



template<typename T = SIM_PREC>
class RndCounter: public SimCounter<T> {
public:
 SIM_CTOR(RndCounter,T,[](T){return rand()*T(2)/RAND_MAX-1;})
}; // RndCounter



template<typename T = SIM_PREC>
class SqWaveCounter: public SimCounter<T> {
public:
 SIM_CTOR(SqWaveCounter,T,[](T t){T x=std::sin(t); return 2*((T(0)<x)-(x<T(0)))-1;})
}; // SqWaveCounter



template<typename T = SIM_PREC>
class ExpCounter: public SimCounter<T> {
public:
 SIM_CTOR(ExpCounter,T,[](T t){return 2*std::exp(-t)-1;})
}; // ExpCounter


template<typename T = SIM_PREC>
class SigmoidCounter: public SimCounter<T> {
public:
 SIM_CTOR(SigmoidCounter,T,[](T t){return 2/(1+std::exp(-t))-1;})
}; // SigmoidCounter
