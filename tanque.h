#ifndef TANQUE_H
#define TANQUE_H

class Tanque
{
public:
    Tanque();
    double getNivelTq1();
    double getNivelTq2();
    double acionaBomba(double sinal);
    void escoaTanque1(double nivelTanque);
    void escoaTanque2(double niveltanque);



private:
    double const K = 10;//cm3/v;
    double const AreaOrificioSaida = 0.17813919765; //cm2
    double const AreaTanque = 15.95179 ;//cm2

    double nivelTq1 = 0;
    double nivelTq2 = 0;
    double nivelMinimo = 1.5;
    double nivelMaximo = 28.0;
    double nivelCritico = 29.5;
};

#endif // TANQUE_H
