#ifndef CONTROLADORPID_H
#define CONTROLADORPID_H

class ControladorPID
{
private:
    double Proporcional;
    double Integrativo;
    double Derivativo;
    double acumulador;
    double erroAnterior;
    //double erroAnterior_I;
public:
    ControladorPID();
    ~ControladorPID() {};

    double ganhoP(double kp, double erro);
    double ganhoI(double ki, double erro);
    double ganhoIWindup(double ki, double taw, double erro,double sinal, double saturacao);
    double ganhoD(double kd, double erro);

    //double ganhoDDerivativo(double kd, double kp, double erro);
    double ganhoPI(double kp, double ki, double erro);
    double ganhoPIWindup(double kp, double ki, double  taw, double erro, double sinal, double saturacao);
    double ganhoPD(double kp, double kd, double erro);
    double ganhoPID(double kp, double ki, double kd, double erro);
    double ganhoPIDWindup(double kp, double ki, double kd, double taw, double erro,double sinal, double saturacao);
    double ganhoPIeD(double kp, double ki, double kd, double erro, double nivel_tanque);
    double ganhoPIeDWindup(double kp, double ki, double kd, double taw, double erro,double sinal,  double nivel_tanque, double saturacao);
    void resetErro();
};

#endif // CONTROLADORPID_H
