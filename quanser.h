// 
// File:   quanser.h
// Author: Leonardo Dantas de Oliveira
//
// Created on 6 de Março de 2008, 20:23
//

#ifndef _QUANSER_H
#define	_QUANSER_H
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using std::cout;
using std::cin;
using std::string;
using namespace std;

class Quanser {
private:
   char* server;
   int tcpPort;
   int    sockfd;
   char   hostaddress[32];
   struct sockaddr_in address;

   

   
   /**
    *Converte de inteiro para std::string
    */
   std::string itoa(int _toConvert){
        std::stringstream ss;
        std::string str;
        ss << _toConvert;
        ss >> str;
        return str;
   }
   /**
    *Converte de float para std::string
    */
   std::string ftoa(float _toConvert){
       std::stringstream ss;
       std::string str;
       ss << _toConvert;
       ss >> str;
       return str;
   }
   
   std::string receiveData() {
      char  ch = ' ';
      std::string _received = "";
      int _count = 0;
      do {
        read(this->sockfd,&ch,1);
        _received.append(1,ch);
        _count++;
      } while (ch != '\n' || _count < 3); //Assumo que nao receberei mensagens menores que 3

      return _received;
   }

   int sendData(std::string _toSend) {
        int _tamanho = _toSend.length();
        write(this->sockfd,_toSend.c_str(),_tamanho);
        return 0;
   }
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
   
public:
   /**
    *Construtor
    */
   Quanser (char* _server, int _tcpPort) {
      this->tcpPort = _tcpPort;
      this->server = _server;
      //this->connectServer();
   }

   /**
    * Tenta conectar ao servidor, se conseguir conectar, retorna 0, se não
    * conseguir, retorna 1
    */
   int connectServer () {
      this->sockfd  = socket(AF_INET, SOCK_STREAM,0);  // criacao do socket
      this->address.sin_family = AF_INET;

      this->address.sin_addr.s_addr = inet_addr(this->server);
      this->address.sin_port = htons(this->tcpPort);

      int len = sizeof(this->address);

      int result = connect(this->sockfd, (struct sockaddr *)
         &this->address, len);

      if (result == -1)  {
         perror ("Houve erro no cliente");
         return 1;
      }
      else {
         return 0;
      }
   }

   /**
    *Grava a tensao especificada no parametro no canal DA 
    */
   int writeDA(int _channel, float _volts) {
        std::string _toSend = "WRITE ";
        _toSend.append(itoa(_channel));
        _toSend.append(" ");
        _toSend.append(ftoa(_volts));
        _toSend.append("\n");
        this->sendData(_toSend);
        std::string _rec = this->receiveData();

        if (_rec.find("ACK",0) > _rec.length() )
            return -1 ; //erro
        else 
            return 0;
   }
   

   /**
    *Le o valor de tensao que esta no canal AD especificado
    */
    double readAD(int _channel) {
        std::string _toSend = "READ ";
        _toSend.append(itoa(_channel));
        _toSend.append("\n");
        this->sendData(_toSend);
        std::string _rec = this->receiveData();
        _rec[1]=',';
        return atof(_rec.c_str());
    }


   /**
    *Destrutor
    */
   ~Quanser(void){
      cout << "Destruido... \n";
      close(this->sockfd);
   }
};




#endif	/* _QUANSER_H */

