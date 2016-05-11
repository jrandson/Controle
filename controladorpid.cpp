#include "controladorpid.h"

ControladorPID::ControladorPID()
{
    Proporcional = 0;
    Integrativo = 0;
    Derivativo = 0;
    acumulador = 0;
    erroAnterior = 0;


}

double ControladorPID::ganhoP(double kp, double erro){
    this->Proporcional = kp*erro;
    return this->Proporcional;
}

double ControladorPID::ganhoI(double ki, double erro){
    this->Integrativo = acumulador+(ki*erro*0.1);
    this->acumulador = this->Integrativo;
    return this->Integrativo;
}

double ControladorPID::ganhoIWindup(double ki, double taw, double erro, double sinal, double saturacao){
    this->Integrativo = acumulador+(ki*erro*0.1);
    this->acumulador = this->Integrativo;
    if(sinal > saturacao){
        double correcao = 0;
        double erro_s;
        erro_s = sinal - saturacao;
        correcao = (1/taw)*erro_s;
        this->Integrativo -= correcao;
        }
    return this->Integrativo;
}

double ControladorPID::ganhoD(double kd, double erro){

    this->Derivativo = kd*((erro - this->erroAnterior)/0.1);
    this->erroAnterior = erro;
    return this->Derivativo;
}

/*double ControladorPID::ganhoDDerivativo(double kd, double kp, double erro){
    double Ts = 0.1;//periodo de amostragem
    double Td = kd/kp; //tempo derivativo
    double poloFiltragem = 0.5;
    this->Derivativo *= 1-(Ts*poloFiltragem/Td); // aplicação do filtro com frequencia de corte
    this->Derivativo -= kp*poloFiltragem*(this->erroAnterior - erro); //aplicacao do termo proporcional no erro
    return this->Derivativo;
}*/

double ControladorPID::ganhoPI(double kp, double ki, double erro){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoI(ki,erro);
    return GP + GI;
}

double ControladorPID::ganhoPIWindup(double kp, double ki, double  taw, double erro, double sinal, double saturacao){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoIWindup(ki, taw, erro, sinal, saturacao);
    return GP + GI;
}

double ControladorPID::ganhoPD(double kp, double kd, double erro){
    double GP = this->ganhoP(kp,erro);
    double GD = this->ganhoD(kd,erro);
    return GP + GD;
}

double ControladorPID::ganhoPID(double kp, double ki, double kd, double erro){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoI(ki,erro);
    double GD = this->ganhoD(kd,erro);
    return GP + GI + GD;
}

double ControladorPID::ganhoPIeD(double kp, double ki, double kd, double erro, double nivel_tanque){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoI(ki,erro);
    double GD = this->ganhoD(kd,nivel_tanque);
    return GP + GI + GD;
}

double ControladorPID::ganhoPIDWindup(double kp, double ki, double kd, double taw, double erro, double sinal, double saturacao){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoIWindup(ki, taw, erro, sinal, saturacao);
    double GD = this->ganhoD(kd,erro);
    return GP + GI + GD;
}

double ControladorPID::ganhoPIeDWindup(double kp, double ki, double kd, double taw, double erro,double sinal, double nivel_tanque, double saturacao){
    double GP = this->ganhoP(kp,erro);
    double GI = this->ganhoIWindup(ki, taw, erro, sinal, saturacao);
    double GD = this->ganhoD(kd,nivel_tanque);
    return GP + GI + GD;
}

/*void ControladorPID::resetAculumador(){
    this->acumulador = 0;
}*/

void ControladorPID::resetErro(){
    //this->erroAnterior = 0;
    this->acumulador = 0;
}


