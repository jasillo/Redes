#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <thread> 
#include <mutex>
 
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
#define FRAMERATE 50000
#define XUNIT 0.9
#define YUNIT 0.4



using namespace std;

int sockfd;
bool isconnected;
string user;
Ground ground;
int posX, posY;
mutex mtxScreen;

// se perdio la connecion
void lostConection(){
    mtxScreen.lock();
        isconnected = false;
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
    mtxScreen.unlock();
    cout<<"se cerro conecion"<<endl;
};

//dibujar campo
void render(){
    clear();
    for (int i=0; i<ground.players.size(); ++i){
        mvprintw((50.0 - ground.y[i])*YUNIT, ground.x[i]*XUNIT, "o");
        if (ground.players[i] == user){
            posX = ground.x[i];
            posY = ground.y[i];
        }
    }
    refresh();
};


// se encarga de recivir los mensajes
void receiver(){
    int r , tam;
    char buffer[BUFFSIZE], headBuffer[10];
    bzero(buffer,BUFFSIZE);
    bzero(headBuffer,10);
    struct PACKET recvPacket;    

    while (isconnected){
        //recivimos tamaño del mensaje
        if (recv (sockfd, headBuffer, USERNAMESIZE + COMMANDSIZE, 0) < 0){        
            lostConection();
            break;
        }
        
        recvPacket.analizeHeader(headBuffer);
        //opciones del header

        if (recvPacket.opt == "m"){ //moverse por el campo
            //leer lo que queda del paquete y dibujar lo necesario
            if (recv (sockfd, buffer, 5, 0) < 0){            
                lostConection();
                break;
            }
            recvPacket.analizeCordinates(buffer);
            
            // modificar mapa

            if (ground.setPosition(recvPacket.user, 
                                recvPacket.corX + recvPacket.dirX, 
                                recvPacket.corY + recvPacket.dirY)){
                if (recvPacket.user == user)
                    posX = recvPacket.corX + recvPacket.dirX;
                    posY = recvPacket.corY + recvPacket.dirY;
                render();
            }
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
        bzero(headBuffer,10);
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
    struct PACKET initialPacket;
    initialPacket.opt = "m";
    initialPacket.user = user;
    initialPacket.corX = 1;
    initialPacket.corY = 1;
    initialPacket.direc = 8;
    string msg = initialPacket.generate();
    
    if (send(servidor, msg.c_str(), msg.size(), 0) < 0)
        lostConection();
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
    posX = posY = 1;
    char ch;

    string u, msg;
    cout<<"ingresar usuario :"<<endl;
    cin>>u; 

    initscr();
    noecho();
    curs_set(FALSE);

    login(u);
    struct PACKET movePacket;
    movePacket.opt = "m";
    movePacket.user = user;    

    while((ch = getch()) != 'x' && isconnected){
        movePacket.corX = posX;
        movePacket.corY = posY;
        if (ch == 'w'){
            movePacket.direc = 0;                        
        }
        else if (ch == 'a'){
            movePacket.direc = 6;
        }
        else if (ch == 's'){
            movePacket.direc = 4;
        }
        else if (ch == 'd'){
            movePacket.direc = 2;
        }        
        else
            continue;

        msg = movePacket.generate();
        if (send(sockfd, msg.c_str(), msg.size(), 0) < 0)
            lostConection();

        usleep(50000);
    }
    endwin();    
    logout();
    
    return 0;
};
 

 

 
