#include "controller.h"

Controller::Controller(char *server, int ip){
    q = new Quanser(server,ip);

}

Controller::~Controller(){

}

double Controller::lerCanal(int canalLeitura){

    double valorCanal = q->readAD(canalLeitura) * 6.25;

    switch (canalLeitura) {
    case 0:
        this->nivelTq1 = valorCanal;
        break;
    case 1:
        this->nivelTq2 = valorCanal;
        break;
    default:
        break;
    }

    return valorCanal;
}

int Controller::conectar()
{
    return q->connectServer();
}

/**
    envia um sinal para a bomba e retorna o sinal enviado

 * @brief Controller::enviaSinal
 * @param canal
 * @param sinal
 */

double Controller::enviaSinal(int canal, double sinal){

    if(sinal > 4.0) sinal = 4.0;
    if(sinal < -4.0) sinal = -4.0;


    if(nivelTq1 <= nivelMinimo && sinal < 0){
        sinal = 0;
    }

    if(nivelTq1 > nivelMaximo && sinal > 10){
        sinal = 0;
    }

    if(nivelTq1 > nivelCritico){
        sinal = 0;
    }

    q->writeDA(canal,sinal);

    return sinal;

}



