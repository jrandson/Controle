#ifndef PAINEL_H
#define PAINEL_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "quanser.h"
#include "controller.h"
#include "QTimer"
#include "QThread"
#include "dlgpid.h"
#include "controladorpid.h"
#include "tanque.h"

namespace Ui {
class Painel;
}

class Painel : public QMainWindow
{
    Q_OBJECT

public:
    explicit Painel(QWidget *parent = 0);
    ~Painel();

    /**
        0.fechada
        1.aberta

        0.degrau
        1.quadrada
        2.senoidal
        3.dente de serra
        4.aleatória
    */

    double T = 1;
    double setPoint = 0,setPoint_ant, offSet = 0;
    int tipoOnda = 1, tipoMalha = 1;
    double erro = 0,erroSecundario = 0;
    double amplitude;
    int pidSecundario = 0;

    int bombaLigada = 1;

    double t = 0, sinal = 0,sinalMestre = 0, sinalSaturado = 0;
    double *valorTeste;
    int canalEscrita = 0, canalLeitura = 0;
    double nivelTq1 = 0, nivelTq2 = 0;
    double nivelTq1_ant,nivelTq2_ant;

    int ordemSistema = 1;


    double sinalOnda;

    int plantaVirtual = 1;

    double tempoSubida;
    double tempoAcomodacao;
    double tempoOvershoot;
    double overshut;
    double epson;


    int flagTempoSubida = 0;
    int flagTempoOvershut = 0;
    int flagTempoRegime = 0;


    double txVariacaoNivel(double nivel, double nivel0);
    void set_tempoSubida();
    void set_tempoOvershoot();
    void set_tempoAcomodacao();


    QCustomPlot *customPlot;
    Controller *controller;


    QTimer *timerEscrita;
    QTimer *timerLeitura;
    QTimer *timerTanqueVirtual;

    QThread *threadTanqueVirtual;
    QThread *thrLeitura;
    QThread *thrEscrita;

    // PID

    ControladorPID *controlePID2;
    ControladorPID *controlePID1;
    int codPid1,codPid2;
    double kd,ki,kp,ti,td, taw;
    double kd2,ki2,kp2,ti2,td2,taw2;

    Tanque *tanque;

    void configEscrita();
    void configLeitura();
    void showtime(double t);
    double getSinalOnda(int tipoOnda);
    double getsinalPID1(double input,double output, double kp,double kd, double ki,double taw);
    double getsinalPID2(double input,double output, double kp,double kd, double ki,double taw);
    void resetTimes();
    void resetaPlanta();
    double getSinalControle();

signals:

public slots:
    void lerDadosPlanta();

    void enviaDadosPlanta();

    void atualizaTanqueVirtual();


private slots:

    void on_rbDegrau_clicked();

    void on_rbQuadrada_clicked();

    void on_rbDenteSerra_clicked();

    void on_rbSenoidal_clicked();


    void on_rbMalhaFechada_clicked();

    void on_rbMalhaAberta_clicked();

    void on_pushButton_2_clicked();

    void on_chbShowErro_clicked(bool checked);

    void on_checkBox_4_clicked(bool checked);


    void on_chbShowSetPointNivel_clicked(bool checked);

    void on_chbSetPointSinal_clicked(bool checked);

    void on_checkBox_5_clicked(bool checked);

    void on_cbCanalEscrita_currentIndexChanged(int index);


    void on_cbPid_currentIndexChanged(int index);


    void on_rbAleatoria_clicked();


    void on_chbErro_clicked(bool checked);

    void on_chbSetPointNivel_clicked(bool checked);

    void on_chbSinalCalculado_clicked(bool checked);

    void on_pushButton_clicked();

    void on_dsbAcomodcao_editingFinished();

    void on_cbOrdem_currentIndexChanged(int index);

    void on_chbTq1_clicked(bool checked);

    void on_chbTq1_2_clicked(bool checked);

    void on_pushButton_3_clicked();

    void on_dsbTi_editingFinished();

    void on_dsbTd_editingFinished();

    void on_checkBox_5_clicked();

    void on_chbAntiwindup_clicked(bool checked);



    void on_cbPid_2_currentIndexChanged(int index);

    void on_pushButton_4_clicked();

    void on_radioButton_clicked();

    void on_checkBox_6_clicked(bool checked);

    void on_chbPidSec_clicked(bool checked);

private:
    Ui::Painel *ui;
};

#endif // PAINEL_H
