#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <thread> 
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <iostream>
#include <algorithm>
#include <ncurses.h>

#include "Packet.h"
 
#define SERVERPORT 8080 
#define BUFFSIZE 1024
#define USERNAMESIZE 2
#define COMMANDSIZE 1

using namespace std;

int sockfd;
bool isconnected;
string user;
Ground ground;

// se perdio la connecion
void lostConection(){
    isconnected = false;
    close(sockfd);
};


// se encarga de recivir los mensajes
void receiver(){
    int r , tam;
    char buffer[BUFFSIZE], headBuffer[4];
    bzero(buffer,BUFFSIZE);
    bzero(headBuffer,4);
    struct PACKET recvPacket;
    int headerSize = USERNAMESIZE + COMMANDSIZE;

    while (isconnected){
        //recivimos tamaño del mensaje
        r = recv (sockfd, headBuffer, headerSize, 0);
        if (r < 0){
            lostConection();
            break;
        }
        
        recvPacket.analizeHeader(headBuffer);
        //opciones del header
        if (recvPacket.opt == "m"){ //moverse por el campo
            //leer lo que queda del paquete y dibujar lo necesario
            r = recv (sockfd, buffer, 5, 0);
            if (r < 0){
                lostConection();
                break;
            }
            recvPacket.analizeCordinates(buffer);

            // modificar mapa
            ground.setPosition(recvPacket.user, recvPacket.corX + recvPacket.dirX, recvPacket.corY + recvPacket.dirY);
            
            // dibujar

        }
        else if (recvPacket.opt == "x"){ // tabla

        }
        else if (recvPacket.opt == "s"){ // disparar

        }
        else if (recvPacket.opt == "c"){ //chat

        }
        else {
            // opcion no reconocida
        }
        

        bzero(buffer,BUFFSIZE);
        bzero(headBuffer,headerSize);
    }

};

//enviar mensajes a todos los usuarios
void sendtoall(string msg) {
        
    if(!isconnected) {
        cout<<"no estas conectado"<<endl;
        return;
    } 
    
    struct PACKET sendPacket;
    sendPacket.opt = "c"; //chat
    sendPacket.user = user;
    sendPacket.message = msg;
    string sendMsg = sendPacket.generate(); 

    int s = send(sockfd, sendMsg.c_str(), sendMsg.size(),0);
    
    if (s < 0){        
        cout<<"error al enviar mensaje"<<endl;
    }
}


// se conecta al servidor
void login(string u) {    
    
    if(isconnected) {
        cout<<"ya estas conectado al servidor"<<endl;        
        return;
    }

    int servidor;         
    string ip = "127.0.0.1";
    struct  sockaddr_in direc; 

    if((servidor = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        cout<<"error al crear el socket"<<endl;
        exit(0);
    }  

    direc.sin_family = AF_INET;
    direc.sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, ip.c_str(), &direc.sin_addr);

    if (connect(servidor,(struct sockaddr *)&direc, sizeof(direc)) == -1 ){
        cout<<"conexion al servidor fallo"<<endl;
        return;
    } 
    sockfd = servidor;
    user = u.substr(0, USERNAMESIZE);
    isconnected = true;
    thread (receiver).detach();

   //enviar posicion inicial
}

//se desconecta del servidor
void logout() {
    
    int sent;
    struct PACKET exitPacket;
    
    if(!isconnected) {
        cout<<"ya estas desconectado"<<endl;
        return;
    }
    isconnected = false;

    exitPacket.opt = "e"; //opcion salir
    exitPacket.user = user;
    exitPacket.message = "";
    string exitMessage = exitPacket.generate();
    
    sent = send(sockfd, exitMessage.c_str(), exitMessage.size(), 0);

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

int main(int argc, char **argv) {   

    char buffer[BUFFSIZE];
    int sockfd, aliaslen; 
    isconnected = false;
    string u;
    
    cout<<"introducir usuario"<<endl;
    cin.getline(buffer, BUFFSIZE);    
    login(buffer);

    cout<<"presionar cualquier tecla para comenzar el juego"<<endl;

    initscr();       
    noecho();
    int ch; 
    while(isconnected) {        
        /*if (kbhit()) {
             
            ch = = getch();
            ver que tecla ha sido presionada
            y enviar el send dependiendo de ello

            
        }*/
    }
    return 0;
};
 

 

 
