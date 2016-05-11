#ifndef TANQUE_H
#define TANQUE_H

class Tanque
{
public:
    Tanque();
    double getNivel();
    double acionaBomba(double sinal);

private:
    double nivel = 0;
    double nivelMinimo = 1.5;
    double nivelMaximo = 28.0;
    double nivelCritico = 29.5;
};

#endif // TANQUE_H

double Tanque::acionaBomba(double sinal){

}

double Tanque::getNivel(){
    return this->nivel;
}
