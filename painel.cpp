#include "painel.h"
#include "ui_painel.h"
#include "qcustomplot.h"
#include "controller.h"
#include "signal.h"
#include "math.h"

/** variaveis auxiliares usadas na geração das ondas dente de serra e random */
double tempoDenteSerra = 0;
double tempoRandom = 0;



// usado para o calculo dos PIDs

// tempo do timer ms
double r;

double windup = 0,windup2 = 0;

//usado para calcular o tempo de overshut
double tx_ant = 0;
double tempoAcomodacaoAux = 0;

//PI-D
double nivel_tanque_aux1,nivel_tanque_aux2;
// antiwindup
double sinal_aux = 0;


Painel::Painel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Painel)
{
    ui->setupUi(this);

    //ui->rbMalhaAberta->click();
    ui->rbDegrau->click();

    this->T = ui->dsbPeriodo->value();
    offSet = ui->dsbOffSet->value();
    this->amplitude = ui->dsbSetPoint->value();

    tipoOnda = 0;
    this->controlePID1 = new ControladorPID();
    this->codPid1 = ui->cbPid->currentIndex();
    this->kd = 0.05;
    this->ki = 0.1;
    this->kp = 2;


    this->codPid2 = ui->cbPid_2->currentIndex();
    this->controlePID2 = new ControladorPID();
    this->kp2 = 2;
    this->ki2 = 0.1;
    this->kd2 = 0.05;


    this->erro = 0;
    this->erroSecundario = 0;

    ui->pbTanque1->setValue(0);
    ui->pbTanque2->setValue(0);

    ui->rbMalhaAberta->click();

    this->tanque = new Tanque();

    //configura timer do tanque virtual
    timerTanqueVirtual= new QTimer(this);
    threadTanqueVirtual = new QThread(0);

    timerTanqueVirtual->moveToThread(threadTanqueVirtual);
    connect(timerTanqueVirtual, SIGNAL(timeout()), this, SLOT(atualizaTanqueVirtual()));

    this->timerTanqueVirtual->start(100);
    this->threadTanqueVirtual->start();

    this->plantaVirtual = 1;

    this->epson = ui->dsbEpson->value();

    this->configEscrita();
    this->configLeitura();
}

Painel::~Painel()
{
    delete ui;
}

double Painel::txVariacaoNivel(double nivel, double nivel0)
{
    return (nivel - nivel0)/0.1;
}

void Painel::set_tempoSubida()
{
    double pv, pv_ant = 0;

    if(this->ordemSistema == 1){
        pv = this->nivelTq1;
        pv_ant = this->nivelTq1_ant;
    }
    else if(this->ordemSistema == 2){
        pv = this->nivelTq2;
        pv_ant = this->nivelTq2_ant;
    }

    //subindo
    if(pv > pv_ant){

        if(pv >= this->setPoint)
            this->flagTempoSubida = 1;


        if(this->flagTempoSubida == 0){
            if(pv < (this->setPoint+this->offSet) ){
                this->tempoSubida += 0.1;                
            }
        }

    }
    else if(pv < pv_ant){

        if(pv <= this->setPoint)
            this->flagTempoSubida = 1;

        if(this->flagTempoSubida == 0){
            if(pv > (this->setPoint+this->offSet) ){
                this->tempoSubida += 0.1;                
            }
        }
    }

    ui->lbTempoSubida->setText("Tempo de subida: " + QString::number(this->tempoSubida) + " s");
}

void Painel::set_tempoOvershoot()
{
    double pv, pv_ant = 0;

    if(this->ordemSistema == 1){
        pv = this->nivelTq1;
        pv_ant = this->nivelTq1_ant;
    }
    else if(this->ordemSistema == 2){
        pv = this->nivelTq2;
        pv_ant = this->nivelTq2_ant;
    }

    double tx = this->txVariacaoNivel(pv,pv_ant);

    if(tx*tx_ant < 0){
        this->flagTempoOvershut = 1;
    }

    if(this->flagTempoOvershut == 0){
        if(tx*tx_ant > 0){
            this->tempoOvershoot += 0.1;

            overshut = (pv-setPoint)*100/setPoint;
            if(overshut < 0)
                overshut *= -1;
        }        
        tx_ant = tx;
    }
    else
        tx_ant = 0;

    ui->lbTempoOverShoot->setText("tempo de Overshoot: " + QString::number(this->tempoOvershoot) + " s");

    if(flagTempoSubida)
        ui->lbOverShoot->setText("OverShoot: " + QString::number(overshut) + " %");

}

void Painel::set_tempoAcomodacao()
{
    double pv, pv_ant = 0;

    if(this->ordemSistema == 1){
        pv = this->nivelTq1;
        pv_ant = this->nivelTq1_ant;
    }
    else{
        pv = this->nivelTq2;
        pv_ant = this->nivelTq2_ant;
    }

    tempoAcomodacaoAux += 0.1;

    if(pv > this->setPoint*(1+this->epson/100)){
        this->tempoAcomodacao += tempoAcomodacaoAux;
        tempoAcomodacaoAux = 0;
    }

    if(pv < this->setPoint*(1-this->epson/100)){
        this->tempoAcomodacao += tempoAcomodacaoAux;
        tempoAcomodacaoAux = 0;
    }

    ui->lbTempoAcomodacao->setText("Tempo de acomodaçao: " + QString::number(this->tempoAcomodacao) + " s");

}

void Painel::configLeitura()
{
    // Nivel do tanque1
    ui->wdgPlotNivel->addGraph();
    ui->wdgPlotNivel->graph(0)->setPen(QPen(Qt::black));
    ui->wdgPlotNivel->graph(0)->setAntialiasedFill(false);
    ui->wdgPlotNivel->graph(0)->setName("Nivel do tanque1");



    // Set Point
    ui->wdgPlotNivel->addGraph();
    ui->wdgPlotNivel->graph(1)->setPen(QPen(Qt::blue));
    ui->wdgPlotNivel->graph(1)->setVisible(true);
    ui->wdgPlotNivel->graph(1)->setName("Set Point");

    // Erro
    ui->wdgPlotNivel->addGraph(); // red line
    ui->wdgPlotNivel->graph(2)->setPen(QPen(Qt::red));
    ui->wdgPlotNivel->graph(2)->setAntialiasedFill(false);
    ui->wdgPlotNivel->graph(2)->setName("Erro");

    //Nivel do tanque2
    ui->wdgPlotNivel->addGraph();
    ui->wdgPlotNivel->graph(3)->setPen(QPen(Qt::green));
    ui->wdgPlotNivel->graph(3)->setAntialiasedFill(false);
    ui->wdgPlotNivel->graph(3)->setName("Nivel do tanque2");


    ui->wdgPlotNivel->legend->setVisible(true);


    ui->wdgPlotNivel->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->wdgPlotNivel->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->wdgPlotNivel->xAxis->setAutoTickStep(false);
    ui->wdgPlotNivel->xAxis->setTickStep(2);


    ui->wdgPlotNivel->yAxis->setRange(0,30);
    ui->wdgPlotNivel->yAxis->setNumberPrecision(2);
    ui->wdgPlotNivel->yAxis->setLabel("Nivel do tanque (Cm) ");
    ui->wdgPlotNivel->xAxis->setLabel("Tempo");

    // make left and bottom axes transfer their ranges to right and top axes:
    // connect(ui->wdgPlotNivel->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->wdgPlotNivel->xAxis2, SLOT(setRange(QCPRange)));
    //connect(ui->wdgPlotNivel->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->wdgPlotNivel->yAxis2, SLOT(setRange(QCPRange)));

     timerLeitura = new QTimer(this);
     thrLeitura = new QThread();

    //configura o timer pra executar a função continuamente
    connect(timerLeitura, SIGNAL(timeout()), this, SLOT(lerDadosPlanta()));

    timerLeitura->moveToThread(thrLeitura);

}

void Painel::lerDadosPlanta()
{
    // calculate two new data points:
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    static double lastPointKey = 0;

    // ler o sinal do servidor aqui

    this->nivelTq1_ant = this->nivelTq1;
    this->nivelTq2_ant = this->nivelTq2;

    if(plantaVirtual == 1){        
        this->nivelTq1 = this->tanque->getNivelTq1();
        this->nivelTq2 = this->tanque->getNivelTq2();
    }
    else{
        this->nivelTq1 = controller->lerCanal(0);
        this->nivelTq2 = controller->lerCanal(1);
    }

    if(this->tipoMalha == 0){
        this->set_tempoSubida();
        this->set_tempoOvershoot();
        this->set_tempoAcomodacao();
    }
    else{
        ui->lbTempoAcomodacao->setText("Tempo de acomodação:");
        ui->lbTempoOverShoot->setText("Tempo de OverShoot:");
        ui->lbOverShoot->setText("OverShoot:");
    }

    double pctTq1 = (this->nivelTq1/30)*100;
    double pctTq2 = (this->nivelTq2/30)*100;

    ui->pbTanque1->setValue(pctTq1);
    ui->pbTanque2->setValue(pctTq2);

    // add data to lines:
    double erro_abs = this->erro;
    if(erro_abs < 0) erro_abs *= -1;

    ui->lbErro->setText("Erro: " + QString::number(erro_abs));

    ui->wdgPlotNivel->graph(0)->addData(key/5, this->nivelTq1);
    ui->wdgPlotNivel->graph(1)->addData(key/5,(this->setPoint));
    ui->wdgPlotNivel->graph(2)->addData(key/5,erro_abs);
    ui->wdgPlotNivel->graph(3)->addData(key/5,this->nivelTq2);

    // remove data of lines that's outside visible range:

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->wdgPlotNivel->xAxis->setRange((key+0.25)/5, 8, Qt::AlignRight);
    ui->wdgPlotNivel->replot();
}

void Painel::configEscrita()
{
    // sinal enviado
    ui->wdgPlotTensao->addGraph(); // blue line
    ui->wdgPlotTensao->graph(0)->setPen(QPen(Qt::blue));
    ui->wdgPlotTensao->graph(0)->setAntialiasedFill(false);
    ui->wdgPlotTensao->legend->setVisible(true);
    ui->wdgPlotTensao->graph(0)->setName("Sinal saturado");



    // sinal calculado
    ui->wdgPlotTensao->addGraph(); // red line
    ui->wdgPlotTensao->graph(1)->setPen(QPen(Qt::red));
    ui->wdgPlotTensao->graph(1)->setAntialiasedFill(false);
    ui->wdgPlotTensao->graph(1)->setName("Sinal calculado");


    // faz a escala do x aparecer como escala de tempo
    ui->wdgPlotTensao->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->wdgPlotTensao->xAxis->setDateTimeFormat("hh:mm:ss");

    //ui->wdgPlotTensao->xAxis->setTickLabelType(QCPAxis::ltNumber);

    ui->wdgPlotTensao->xAxis->setAutoTickStep(false);
    ui->wdgPlotTensao->xAxis->setTickStep(2);
    ui->wdgPlotTensao->axisRect()->setupFullAxesBox();

    ui->wdgPlotTensao->yAxis->setRange(-12,12);
    ui->wdgPlotTensao->yAxis->setNumberPrecision(1);
    ui->wdgPlotTensao->yAxis->setLabel("Tensao (v) ");
    ui->wdgPlotTensao->xAxis->setLabel("Tempo");

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->wdgPlotTensao->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->wdgPlotTensao->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->wdgPlotTensao->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->wdgPlotTensao->yAxis2, SLOT(setRange(QCPRange)));

     timerEscrita= new QTimer(this);
     thrEscrita = new QThread(0);

    //configura o timer pra executar a função continuamente
    connect(timerEscrita, SIGNAL(timeout()), this, SLOT(enviaDadosPlanta()));

    timerEscrita->moveToThread(thrEscrita);

}

double Painel::getSinalOnda(int tipoOnda)
{
    double onda;
    Signal *signal;

    switch (tipoOnda) {
        case 0: // degrau
            onda = signal->degrau();
            break;
        case 1:// quadrada
            onda = signal->ondaQuadrada(t,T);

            break;
        case 2:// dente de serra
            onda = signal->denteSerra(tempoDenteSerra,T);
            break;
        case 3:// senoide
            onda = signal->senoide(t,T);
            break;
        case 4:
            if(tempoRandom > T){
                r = signal->randomSignal();
                tempoRandom = 0;
            }
            onda = r;
            break;
        default:
            break;
    }


    return onda;
}

void Painel::resetTimes(){

    if(setPoint != setPoint_ant){
        setPoint_ant = setPoint;

        ui->lbOverShoot->setText("OverShoot: ");

        this->tempoSubida = 0;
        this->tempoOvershoot = 0;
        this->tempoAcomodacao = 0;

        this->flagTempoOvershut = 0;
        this->flagTempoRegime = 0;
        this->flagTempoSubida = 0;

        tempoAcomodacaoAux = 0;

    }
}

void Painel::resetaPlanta()
{
    this->T = ui->dsbPeriodo->value();
    offSet = ui->dsbOffSet->value();
    this->amplitude = ui->dsbSetPoint->value();

    tipoOnda = 0;
    this->controlePID1 = new ControladorPID();
    this->codPid1 = ui->cbPid->currentIndex();
    this->kd = 0.05;
    this->ki = 0.1;
    this->kp = 2;

    this->codPid2 = ui->cbPid_2->currentIndex();
    this->controlePID2 = new ControladorPID();
    this->kp2 = 2;
    this->ki2 = 0.1;
    this->kd2 = 0.05;


    this->erro = 0;
    this->erroSecundario = 0;

    this->nivelTq1 = this->nivelTq1_ant = 0;
    this->nivelTq2 = this->nivelTq2_ant = 0;

    ui->pbTanque1->setValue(0);
    ui->pbTanque2->setValue(0);
}


void Painel::enviaDadosPlanta()
{
    // calculate two new data points:
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    static double lastPointKey = 0;

    this->sinal = this->getSinalControle();

    if(this->bombaLigada == 0){
        this->sinal = 0;
        this->setPoint = 0;
    }

    if(this->plantaVirtual == 1){
        this->sinalSaturado = tanque->acionaBomba(this->sinal);
    }
    else{
        this->sinalSaturado = controller->enviaSinal(0,this->sinal);
    }

    ui->lbSinalEnviado->setText("Sinal: " + QString::number(sinal) + " v");

    ui->lbTime->setText("Time: " + QString::number(t));

    ui->lnEdtNivelTq1->setText(QString::number(this->nivelTq1));
    ui->lnEdtNivelTq2->setText(QString::number(this->nivelTq2));

    // add data to lines:
    ui->wdgPlotTensao->graph(0)->addData(key/5, this->sinalSaturado);
    ui->wdgPlotTensao->graph(1)->addData(key/5,this->sinal);

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->wdgPlotTensao->xAxis->setRange((key + 0.25)/5, 10, Qt::AlignRight);


    ui->wdgPlotTensao->replot();

    this->t += 0.1;

    tempoDenteSerra += 0.1;
    if(tempoDenteSerra > T)
        tempoDenteSerra = 0;

    tempoRandom += 0.1;
}

double Painel::getSinalControle()
{
    double sinalControle = 0;
    double sinalMestreControle = 0;

    this->sinalOnda = this->getSinalOnda(this->tipoOnda);

    // se a malha for fechada...
    if(tipoMalha == 0){

        this->setPoint = this->amplitude * sinalOnda + this->offSet;

        this->resetTimes();

        if(this->setPoint < 0) this->setPoint = 0;

        //controla o tanque 1.
        if(this->ordemSistema == 1){
            nivel_tanque_aux1 = this->nivelTq1;

            this->erro = setPoint - this->nivelTq1;

            sinalControle = this->getsinalPID1(this->setPoint,this->nivelTq1,this->kp,this->kd,this->ki,this->taw);

        }// controla o tanque 2.
        else if(this->ordemSistema == 2){

            nivel_tanque_aux1 = this->nivelTq1;

            nivel_tanque_aux2 = this->nivelTq2;

            this->erro = this->setPoint - this->nivelTq2;

            //usa PID escravo
            if(ui->chbPidSec->isChecked()){

                sinalMestreControle = this->getsinalPID1(this->setPoint,this->nivelTq2,this->kp2,this->kd2,this->ki2,this->taw2);

                sinalControle = this->getsinalPID2(sinalMestreControle,this->nivelTq1,this->kp,this->kd,this->ki,this->taw);
            }
            else{

                sinalControle = this->getsinalPID1(this->setPoint,this->nivelTq2,this->kp,this->kd,this->ki,this->taw);

            }
        }
    }
    else{
        sinalControle = this->amplitude * sinalOnda + offSet;
    }


    return sinalControle;
}


double Painel::getsinalPID1(double input,double output, double kp,double kd, double ki,double taw)
{
    double sinal;
    double erro = input - output;

    switch(codPid1){
    case 0: // P
        sinal = controlePID1->ganhoP(kp,erro);
        break;
    case 1:// PI
        if(windup == 1){
            sinal = controlePID1->ganhoIWindup(ki, this->taw, erro,sinal_aux,29.8);
            sinal_aux = this->sinal;
        }
        else
            sinal = controlePID1->ganhoPI(kp,ki,erro);
        break;
    case 2: //PD
        sinal = controlePID1->ganhoPD(this->kp, this->kd, erro);
        break;
    case 3: // PID
        if(windup == 1){
            sinal = controlePID1->ganhoPIDWindup(this->kp,this->ki,this->kd,t,erro,sinal_aux,29.8);
            sinal_aux = this->sinal;
        }
        else
            this->sinal = controlePID1->ganhoPID(kp,ki,kd,erro);
        break;
    case 4: // PI-D

        if(windup == 1){
            sinal = controlePID1->ganhoPIeDWindup(kp,ki,kd,taw,erro,sinal_aux,nivel_tanque_aux1,29.8);
            sinal_aux = this->sinal;
        }
        else
            sinal = controlePID1->ganhoPIeD(kp,ki,kd,erro,nivel_tanque_aux1);
        break;
    }

    return sinal;
}


//rever as variáveis desta função: sinal_aux
double Painel::getsinalPID2(double input,double output, double kp,double kd, double ki,double taw)
{
    double sinal;
    double erro;

    erro = input - output;

    switch(codPid2){
    case 0: // P
        sinal = controlePID2->ganhoP(kp,erro);
        break;
    case 1:// PI
        if(windup == 1){
            sinal = controlePID2->ganhoIWindup(ki, this->taw, erro,sinal_aux,29.8);
            sinal_aux = this->sinal;
        }
        else
            sinal = controlePID2->ganhoPI(kp,ki,erro);
        break;
    case 2: //PD
        sinal = controlePID2->ganhoPD(this->kp, this->kd, erro);
        break;
    case 3: // PID
        if(windup == 1){
            sinal = controlePID2->ganhoPIDWindup(this->kp,this->ki,this->kd,t,erro,sinal_aux,29.8);
            sinal_aux = this->sinal;
        }
        else
            this->sinal = controlePID2->ganhoPID(kp,ki,kd,erro);
        break;
    case 4: // PI-D

        if(windup == 1){
            sinal = controlePID2->ganhoPIeDWindup(kp,ki,kd,taw,erro,sinal_aux,nivel_tanque_aux2,29.8);
            sinal_aux = this->sinal;
        }
        else
            sinal = controlePID2->ganhoPIeD(kp,ki,kd,erro,nivel_tanque_aux2);
        break;
    }

    return sinal;
}



void Painel::atualizaTanqueVirtual()
{
    this->tanque->escoaTanque1(this->nivelTq1);
    this->tanque->escoaTanque2(this->nivelTq2);
}

/*** controle dos botões do tipo de onda*/

void Painel::on_rbDegrau_clicked()
{
    this->tipoOnda = 0;
    this->offSet = 0;
    ui->dsbOffSet->setValue(0);
    this->sinal = ui->dsbSetPoint->value();
    ui->dsbSetPoint->setEnabled(true);
    ui->dsbPeriodo->setEnabled(false);
    ui->dsbOffSet->setEnabled(false);

}

void Painel::on_rbQuadrada_clicked()
{
    this->tipoOnda = 1;
    sinal = ui->dsbSetPoint->value();
    ui->dsbPeriodo->setEnabled(true);
    ui->dsbOffSet->setEnabled(true);
}

void Painel::on_rbDenteSerra_clicked()
{
    this->tipoOnda = 2;
    ui->dsbPeriodo->setEnabled(true);
    ui->dsbOffSet->setEnabled(true);
}

void Painel::on_rbSenoidal_clicked()
{
    this->tipoOnda = 3;
    ui->dsbPeriodo->setEnabled(true);
    ui->dsbOffSet->setEnabled(true);
}

void Painel::on_rbAleatoria_clicked()
{
    this->tipoOnda = 4;
    r = 1;
    ui->dsbPeriodo->setEnabled(true);
    ui->dsbOffSet->setEnabled(false);
}



/** fim dos controles do botões do tipo de onda*/

void Painel::on_rbMalhaFechada_clicked()
{
    ui->dsbSetPoint->setMaximum(30);
    ui->dsbSetPoint->setMinimum(0);

    ui->lb1->setText("Amplitude (Cm)");
    this->tipoMalha = 0;


    ui->lbOffSet->setText("Off Set (Cm)");

    ui->dsbOffSet->setMinimum(0);
    ui->dsbOffSet->setMaximum(20);

    this->setPoint = ui->dsbSetPoint->value();

    ui->chbErro->setChecked(true);
    ui->chbErro->setEnabled(true);

    //ui->wdgPlotNivel->graph(1)->setVisible(false);
    //ui->wdgPlotNivel->graph(2)->setVisible(false);

    ui->chbSetPointNivel->setChecked(true);
    ui->chbSetPointNivel->setEnabled(true);

    this->setPoint = ui->dsbSetPoint->value();

    this->codPid1 = this->codPid2 = 0;

    ui->frmPID1->setEnabled(true);
     ui->pbAtualizaPID1->setEnabled(true);

    ui->cbOrdem->setEnabled(true);

    switch(ui->cbOrdem->currentIndex()){
        case 0:
            this->ordemSistema = 1;
        break;
        case 1:
            this->ordemSistema = 2;
        break;
    }
}

void Painel::on_rbMalhaAberta_clicked()
{
    ui->dsbSetPoint->setMaximum(4);
    ui->dsbSetPoint->setMinimum(-4);

    ui->lb1->setText("Sinal (v)");
    ui->lbSetPoint->setText("");

    this->tipoMalha = 1;

    ui->lbOffSet->setText("Off Set (v)");

    ui->dsbOffSet->setMinimum(-3);
    ui->dsbOffSet->setMaximum(3);

    this->sinal = ui->dsbSetPoint->value();

    ui->chbErro->setChecked(false);
    ui->chbErro->setEnabled(false);

    ui->chbSetPointNivel->setChecked(false);
    ui->chbSetPointNivel->setEnabled(false);

    this->sinal = ui->dsbSetPoint->value();

    ui->frmPID1->setEnabled(false);
    ui->pbAtualizaPID1->setEnabled(false);

    ui->cbOrdem->setEnabled(false);

    ui->chbPidSec->setEnabled(false);
    this->pidSecundario = 0;



}

/**
 * @brief Painel::on_pushButton_2_clicked
 * Conexão com o servidor
 */
void Painel::on_pushButton_2_clicked()
{


    try{
        controller = new Controller("10.13.99.69", 20081);

        int connectionStatus = 1;

        if(plantaVirtual == 1){
            ui->lbStauxConexao->setText("Planta virtual");
            connectionStatus = 0;
        }
        else{
             connectionStatus = controller->conectar();

             if(connectionStatus == 0)
                 ui->lbStauxConexao->setText("Conectado");
        }

        if(connectionStatus == 0){

            this->timerTanqueVirtual->start(100);
            this->threadTanqueVirtual->start();

            this->timerEscrita->start(100);
            this->thrEscrita->start();

            this->timerLeitura->start(100);
            this->thrLeitura->start();
        }
        else
        if(connectionStatus == 1){
            ui->lbStauxConexao->setText("A conexao falhou ");
        }



    }
    catch(int f){
        return;
    }
}


void Painel::on_chbShowErro_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(2)->setVisible(checked);
}



void Painel::on_checkBox_4_clicked(bool checked)
{
    if(checked){
        this->erro = 0;
        this->bombaLigada = 1;
    }
    else{
        this->bombaLigada = 0;


    }
}

void Painel::on_chbShowSetPointNivel_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(1)->setVisible(checked);
}

void Painel::on_chbSetPointSinal_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(1)->setVisible(checked);
}



/* muda de planta virtual para real e vice versa**/

void Painel::on_checkBox_5_clicked(bool checked)
{
    if(checked){
        plantaVirtual == 1;
        this->timerTanqueVirtual->start(100);
        this->threadTanqueVirtual->start();
    }
    else{
        this->timerTanqueVirtual->stop();

        plantaVirtual = 0;
    }

    this->resetaPlanta();
}


void Painel::on_cbCanalEscrita_currentIndexChanged(int index)
{
    canalEscrita = index;
}

void Painel::on_cbPid_currentIndexChanged(int index)
{
    switch(index){
    case 0: // P
        ui->dsbKi->setEnabled(false);
        ui->dsbKp->setEnabled(true);
        ui->dsbKd->setEnabled(false);
        ui->dsbTd->setEnabled(false);
        ui->dsbTi->setEnabled(false);
        ui->chbAntiwindup->setEnabled(false);
        ui->chbAntiderivativo->setEnabled(false);
        windup = 0;
        break;
    case 1:// PI
        ui->dsbKi->setEnabled(true);
        ui->dsbKp->setEnabled(true);
        ui->dsbKd->setEnabled(false);
        ui->dsbTd->setEnabled(false);
        ui->dsbTi->setEnabled(false);
        ui->chbAntiwindup->setEnabled(true);
        ui->chbAntiderivativo->setEnabled(false);
        windup = 0;
        break;
    case 2: //PD
        ui->dsbKi->setEnabled(false);
        ui->dsbKp->setEnabled(true);
        ui->dsbKd->setEnabled(true);
        ui->dsbTd->setEnabled(false);
        ui->dsbTi->setEnabled(false);
        ui->chbAntiwindup->setEnabled(false);
        ui->chbAntiderivativo->setEnabled(true);
        windup = 0;
        break;
    case 3: // PID
        ui->dsbKi->setEnabled(true);
        ui->dsbKp->setEnabled(true);
        ui->dsbKd->setEnabled(true);
        ui->dsbTd->setEnabled(true);
        ui->dsbTi->setEnabled(true);
        ui->chbAntiwindup->setEnabled(true);
        ui->chbAntiderivativo->setEnabled(true);
        windup = 1;
        break;
    case 4: // PI-D
        ui->dsbKi->setEnabled(true);
        ui->dsbKp->setEnabled(true);
        ui->dsbKd->setEnabled(true);
        ui->dsbTd->setEnabled(true);
        ui->dsbTi->setEnabled(true);
        ui->dsbTaw->setEnabled(true);
        ui->chbAntiwindup->setEnabled(true);
        ui->chbAntiderivativo->setEnabled(true);
        windup = 1;
        break;
    }

    this->codPid1 = ui->cbPid->currentIndex();
}




void Painel::on_chbErro_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(2)->setVisible(checked);
}

void Painel::on_chbSetPointNivel_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(1)->setVisible(checked);
}

void Painel::on_chbSinalCalculado_clicked(bool checked)
{
    ui->wdgPlotTensao->graph(1)->setVisible(checked);
}

/**Atualiza os parâmetros de controle*/
void Painel::on_pushButton_clicked()
{
    this->amplitude = ui->dsbSetPoint->value();

    offSet = ui->dsbOffSet->value();

    if(ui->dsbPeriodo->value()== 0)
       ui->dsbPeriodo->setValue(5);

    this->T = ui->dsbPeriodo->value();
    if(this->tipoMalha == 0)
        ui->lbSetPoint->setText("Set point: " + QString::number(ui->dsbSetPoint->value()) + " Cm");


    this->overshut = 0;
    this->tempoAcomodacao = 0;
    this->tempoOvershoot = 0;

    ui->lbOverShoot->setText("OverShoot: ");
    this->flagTempoOvershut = 0;
    this->flagTempoRegime = 0;
    this->flagTempoSubida = 0;

}

void Painel::on_dsbAcomodcao_editingFinished()
{
    this->epson = ui->dsbEpson->value();
}



void Painel::on_cbOrdem_currentIndexChanged(int index)
{
    switch(index){
        case 0:
            this->ordemSistema = 1;
            ui->chbPidSec->setEnabled(false);
            ui->chbPidSec->setChecked(false);
            this->pidSecundario = 0;
        break;
        case 1:
            this->ordemSistema = 2;
            ui->chbPidSec->setEnabled(true);
            ui->chbPidSec->setChecked(false);
            this->pidSecundario = 1;
        break;
    }
}



void Painel::on_chbTq1_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(0)->setVisible(checked);
}

void Painel::on_chbTq1_2_clicked(bool checked)
{
    ui->wdgPlotNivel->graph(3)->setVisible(checked);
}

/**Atualiza parâmetros do PID1*/
void Painel::on_pushButton_3_clicked()
{
    this->kp = ui->dsbKp->value();
    this->kd = ui->dsbKd->value();

    this->taw = ui->dsbTaw->value();

    if(ui->dsbKi->value() != 0){
        this->ki = ui->dsbKi->value();
        this->ti = this->kp/this->ki;
        ui->dsbTi->setValue(this->ti);
    }
    else{
        QMessageBox::information(this,"Erro", "Ki não pode ser zero");
        ui->dsbKi->clear();
        ui->dsbKi->setFocus();
    }

    controlePID1->resetErro();

}

void Painel::on_dsbTi_editingFinished()
{

    this->ti = ui->dsbTi->value();

    if(this->ti != 0){
        this->ki = this->kp/this->ti;
        ui->dsbKi->setValue(this->ki);
    }
    else{
        QMessageBox::information(this,"Erro","Ti não pode ser zero");
        ui->dsbTi->clear();
        ui->dsbTi->setFocus();
    }
}

void Painel::on_dsbTd_editingFinished()
{

}

void Painel::on_checkBox_5_clicked()
{

}

void Painel::on_chbAntiwindup_clicked(bool checked)
{
    if(checked){
        windup = 1;
        ui->dsbTaw->setEnabled(true);
    }
    else{
        windup = 0;
        ui->dsbTaw->setEnabled(false);

    }
}


void Painel::on_cbPid_2_currentIndexChanged(int index)
{
    switch (index) {
    case 0: // P
        ui->dsbKp_2->setEnabled(true);
        ui->dsbKd_2->setEnabled(false);
        ui->dsbKi_2->setEnabled(false);

        ui->dsbTaw_2->setEnabled(false);
        ui->dsbTd_2->setEnabled(false);
        ui->dsbTi_2->setEnabled(false);
        ui->chbAntiwindup_2->setEnabled(false);
        ui->chbAntiderivativo_2->setEnabled(false);
        windup2 = 0;
        break;
    case 1: // PI
        ui->dsbKp_2->setEnabled(true);
        ui->dsbKd_2->setEnabled(false);
        ui->dsbKi_2->setEnabled(true);

        ui->dsbTaw_2->setEnabled(true);
        ui->dsbTd_2->setEnabled(false);
        ui->dsbTi_2->setEnabled(true);
        ui->chbAntiwindup_2->setEnabled(true);
        ui->chbAntiderivativo_2->setEnabled(false);
        windup2 = 0;
        break;
    case 2: // PD
        ui->dsbKp_2->setEnabled(true);
        ui->dsbKd_2->setEnabled(true);
        ui->dsbKi_2->setEnabled(false);

        ui->dsbTaw_2->setEnabled(false);
        ui->dsbTd_2->setEnabled(true);
        ui->dsbTi_2->setEnabled(false);
        ui->chbAntiwindup_2->setEnabled(false);
        ui->chbAntiderivativo_2->setEnabled(true);
        windup2 = 0;
        break;
    case 3: // PID
        ui->dsbKp_2->setEnabled(true);
        ui->dsbKd_2->setEnabled(true);
        ui->dsbKi_2->setEnabled(true);

        ui->dsbTaw_2->setEnabled(true);
        ui->dsbTd_2->setEnabled(true);
        ui->dsbTi_2->setEnabled(true);
        ui->chbAntiwindup_2->setEnabled(true);
        ui->chbAntiderivativo_2->setEnabled(true);
        windup2 = 1;
        break;
    case 4: // PI-D
        ui->dsbKp_2->setEnabled(true);
        ui->dsbKd_2->setEnabled(false);
        ui->dsbKi_2->setEnabled(false);

        ui->dsbTaw_2->setEnabled(false);
        ui->dsbTd_2->setEnabled(false);
        ui->dsbTi_2->setEnabled(false);
        ui->chbAntiwindup_2->setEnabled(false);
        ui->chbAntiderivativo_2->setEnabled(false);
        windup2 = 1;
        break;
    default:
        break;
    }
}

/**Atualiza parametros do PID2*/
void Painel::on_pushButton_4_clicked()
{
    this->kp2 = ui->dsbKp_2->value();
    this->kd2 = ui->dsbKd_2->value();

    this->taw2 = ui->dsbTaw_2->value();

    if(ui->dsbKi_2->value() != 0){
        this->ki2 = ui->dsbKi_2->value();
        this->ti2 = this->kp2/this->ki2;
        ui->dsbTi_2->setValue(this->ti2);
    }
    else{
        QMessageBox::information(this,"Erro", "Ki não pode ser zero");
        ui->dsbKi_2->clear();
        ui->dsbKi_2->setFocus();
    }

    controlePID2->resetErro();
}

void Painel::on_radioButton_clicked()
{


}

void Painel::on_checkBox_6_clicked(bool checked)
{
    if(checked)
        this->pidSecundario = 1;
    else
        this->pidSecundario = 0;
}

void Painel::on_chbPidSec_clicked(bool checked)
{
    if(!checked){
        ui->frmPID_2->setEnabled(false);
        ui->pbAtualizaPID2->setEnabled(false);
    }
    else{
        ui->frmPID_2->setEnabled(true);
        ui->pbAtualizaPID2->setEnabled(true);
    }
}
