#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <time.h>
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
#include <vector>
#include <queue> 

#include "Packet.h"
 
#define SERVERPORT 8080 
#define BUFFSIZE 1024
#define USERNAMESIZE 2
#define COMMANDSIZE 1
#define FRAMERATE 50000
#define XUNIT 2
#define YUNIT 1
#define GROUNDSIZE 20


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
    //dibujar bordes
    mvprintw(0, 0, "+");
    mvprintw(GROUNDSIZE, 0, "+");
    mvprintw(0, GROUNDSIZE * XUNIT, "+");
    mvprintw(GROUNDSIZE, GROUNDSIZE * XUNIT, "+");
    for (int i=1; i<GROUNDSIZE * XUNIT; ++i){
        mvprintw(0, i, "-");
        mvprintw(GROUNDSIZE, i, "-");
    }
    for (int i=1; i<GROUNDSIZE ; ++i){
        mvprintw(i, 0, "|");
        mvprintw(i, GROUNDSIZE* XUNIT, "|");
    }  

    //dibujar disparos
    int nextX, nextY;
    for (int i=0; i<ground.shotX.size(); ++i){
        nextX = ground.shotX[i];
        nextY = ground.shotY[i];
        while (nextX>0 && nextY>0 && nextX<GROUNDSIZE*XUNIT && nextY<GROUNDSIZE*YUNIT){
            mvprintw(GROUNDSIZE - nextY, nextX, "x");
            nextX += ground.direX[i];
            nextY += ground.direY[i];
        }
    }

    //dibujar jugadores    
    for (int i=0; i<ground.players.size(); ++i){
        mvprintw((GROUNDSIZE - ground.y[i])*YUNIT, ground.x[i]*XUNIT, "O");
        if (ground.players[i] == user){
            posX = ground.x[i];
            posY = ground.y[i];
        }
    }  
    refresh();
};

void timerShots(){
    clock_t umbral;
    while (isconnected){
        umbral = clock();
        if (!ground.startTime.empty() && double(umbral - ground.startTime.front()) > 500){
            mtxScreen.lock();
                ground.shotX.erase (ground.shotX.begin());
                ground.shotY.erase (ground.shotY.begin());
                ground.direX.erase (ground.direX.begin());
                ground.direY.erase (ground.direY.begin());
                ground.startTime.erase (ground.startTime.begin());
            mtxScreen.unlock();
            render();
        }
        
        usleep(100000);
    }
}


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
            if (recvPacket.direc == 8){
                ground.setPositionNewPlayer(recvPacket.user, recvPacket.corX, recvPacket.corY);
                render();
            }
            else if (ground.setPosition(recvPacket.user, 
                                recvPacket.corX + recvPacket.dirX, 
                                recvPacket.corY + recvPacket.dirY)){
                if (recvPacket.user == user)
                    posX = recvPacket.corX + recvPacket.dirX;
                    posY = recvPacket.corY + recvPacket.dirY;
                render();
            }
        }
        else if (recvPacket.opt == "s"){ // tabla
            if (recv (sockfd, buffer, 5, 0) < 0){            
                lostConection();
                break;
            }
            recvPacket.analizeCordinates(buffer);

            ground.shotX.push_back(recvPacket.corX * XUNIT);
            ground.shotY.push_back(recvPacket.corY * YUNIT);
            ground.direX.push_back(recvPacket.dirX * XUNIT);
            ground.direY.push_back(recvPacket.dirY * YUNIT);
            ground.startTime.push_back(clock());
            render();
        }
        else if (recvPacket.opt == "x"){ // disparar

        }
        else if (recvPacket.opt == "c"){ //chat

        }
        else if (recvPacket.opt == "l"){ //login
            if (recv (sockfd, buffer, 5, 0) < 0){            
                lostConection();
                break;
            }
            recvPacket.analizeCordinates(buffer);
            ground.setPositionNewPlayer(recvPacket.user, recvPacket.corX, recvPacket.corY);
            render();
        }
        else if (recvPacket.opt == "e"){ //chat
            for (int i=0; i<ground.players.size(); ++i){
                if (ground.players[i] == recvPacket.user){
                    ground.players.erase (ground.players.begin()+i);
                    ground.x.erase(ground.x.begin()+i);
                    ground.y.erase(ground.y.begin()+i);
                    //break;
                }
            }
            render();
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
    thread (timerShots).detach();

   //enviar posicion inicial
    struct PACKET initialPacket;
    initialPacket.opt = "l";
    initialPacket.user = user;

    initialPacket.corX = 0 ;
    initialPacket.corY = rand() % (GROUNDSIZE -1 ) +1;
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
    srand (time(NULL));

    string u, msg;
    cout<<"ingresar usuario :"<<endl;
    cin>>u; 
    WINDOW *my_win;
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);    

    login(u);
    struct PACKET movePacket;
    movePacket.opt = "m";
    movePacket.user = user;    

    while((ch = getch()) != 'x' && isconnected){
        movePacket.corX = posX;
        movePacket.corY = posY;

        //movimiento  del jugador
        if (ch == 'w'){ //arriba
            movePacket.opt = "m";
            movePacket.direc = 0;                        
        }
        else if (ch == 'a'){ //izquierda
            movePacket.opt = "m";
            movePacket.direc = 6;
        }
        else if (ch == 's'){ //abajo
            movePacket.opt = "m";
            movePacket.direc = 4;
        }
        else if (ch == 'd'){ //derecha
            movePacket.opt = "m";
            movePacket.direc = 2;
        }
        //opciones de disparo en diagonales
        else if (ch == 'h'){ //arriba-izquierda 
            movePacket.opt = "s";
            movePacket.direc = 7;
        }
        else if (ch == 'j'){ //arriba-derecha
            movePacket.opt = "s";
            movePacket.direc = 1;
        }
        else if (ch == 'k'){ //abajo-izquierda
            movePacket.opt = "s";
            movePacket.direc = 5;
        }
        else if (ch == 'l'){ //abajo-derecha
            movePacket.opt = "s";
            movePacket.direc = 3;
        }                
        else
            continue;

        msg = movePacket.generate();
        if (send(sockfd, msg.c_str(), msg.size(), 0) < 0)
            lostConection();

        usleep(100000);
    }
    
    endwin();    
    logout();
    
    return 0;
};

 
