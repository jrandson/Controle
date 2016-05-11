#ifndef SIGNAL_H
#define SIGNAL_H

#include "qmath.h"

class Signal
{
public:
    Signal();
    double tempoAux = 0;




    //sinais
    double degrau();
    double denteSerra(double t, double T);
    double senoide(double t, double T);
    double ondaQuadrada(double t, double T);
    double randomSignal();
    double senoideLouca(double t);

    int tempo = 0;

};

double Signal::degrau(){
    return 1;

}

double Signal::denteSerra(double t, double T)
{
    double alfa = 2.0/T;

    double sinal = alfa*t -1;

    return sinal;

}

double Signal::senoide(double t, double T)
{
    double pi = 3.14159265359;
    double f = pi/T;
    double signal = sin(f*t);

    return signal;
}

double Signal::ondaQuadrada(double t,double T)
{
    double s = senoide(t,T);

    if(s < 0)
        s = -1;
    else
        s = 1;

    return s;
}

double Signal::randomSignal(){

   double sinal = -10 + 2*(rand()%10+1);

   return sinal/10;
}

double Signal::senoideLouca(double t)
{

    double signalValue = qSin(0.5*t) + qCos(0.8*t) + cos(1.2*t) + qSin(0.1*t);

    return signalValue;
}

#endif // SIGNAL_H
