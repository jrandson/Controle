#ifndef ESTADOS_H
#define ESTADOS_H

#include <qmath.h>



class Estados
{
    public:
        double A1,A2;
        int L20,L10;
        double a1,a2;        
        double alfa1,alfa2,alfa3,beta;

        const double Km = 4.6;
        const double g = 9.82;
        const double T = 0.1;

        double G[2][2];
        double H1,H2;

        Estados();
        double getNivelL1();
        double getNivelL2();
        void setG();
        void setH();

};

#endif // ESTADOS_H


Estados::Estado()
{
    A1 = A2 = 15.5179;
    a1 = a2 = 0.17813919765;

    L20 = 15;
    L10 = L20 * pow(a2,2)/pow(a1,2);

    this->alfa1 = -1*(a1/A1)*sqrt(g/(2*L10));
    this->alfa2 = -1*(a2/A2)*sqrt(g/(2*L20));
    this->alfa3 = (a1/A2)*sqrt(g/(2*L10));

}

double Estados::getNivelL1()
{

}

void Estados::setG()
{
    this->G[0][0] = exp(-1*this->alfa1*T);
    this->G[0][1] = 0;
    this->G[1][0] = exp(-1*alfa2*T) * alfa3/(alfa1 - alfa2) + exp(-1*alfa1*T) * alfa3/(alfa2 - alfa1);
    this->G[1][1] =exp(-1*alfa2*T);
}

void Estados::setH()
{
    double x = exp(-1*alfa2*T) * alfa3/(alfa1 - alfa2) + exp(-1*alfa1*T) * alfa3/(alfa2 - alfa1);
    this->H1 = -1*(beta/alfa1) * exp(-1*alfa1*T);
    this->H2 = x * beta*T;
}
