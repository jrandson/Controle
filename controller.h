#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "quanser.h"
#include "string"

class Controller
{
public:

    explicit Controller(char *server, int ip);

    ~Controller();

    double const nivelMinimo = 2;
    double const nivelMaximo = 28.5;
    double const nivelCritico = 29.5;

    Quanser *q;

    double enviaSinal(int canal, double sinal);
    double lerCanal(int canal);
    void desligaBomba();

    int conectar();

private:
    double nivelTq1 = 0,nivelTq2 = 0;


protected:

    void teste();

};


#endif // CONTROLLER_H
