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

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <mutex> 

#include "Packet.h"

#define PORT 8080 
#define BUFFSIZE 1024
#define USERNAMESIZE 2
#define COMMANDSIZE 1
#define GROUNDSIZE 20

using namespace std;

vector<int> clientsFD;
 
int sockfd;
bool isconnected;
mutex mtxScreen;

void closeConection(int fd){
    mtxScreen.lock();
    for (int i=0; i<clientsFD.size(); ++i){
        if (fd == clientsFD[i]){
            shutdown(fd, SHUT_RDWR);
            close(fd);
            clientsFD.erase (clientsFD.begin()+i);
            break;
        }
    }
    cout<<"conexion cliente cerrada"<<endl;
    mtxScreen.unlock();     
}

// interfaz del servidor que lee commandos de la pantalla (thread)
void handler(){
    char buffer[BUFFSIZE]; 
    
    while(cin.getline(buffer, BUFFSIZE)) {
        //analizar linea de comando dentro de packet
        string line = buffer;
        string option = getWord(&line);        
        // ejecutar la opcion elegida
        if (option == "quit"){            
            isconnected = false;
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);;
            break;
        }       
        else{
            mtxScreen.lock();
            cout<<"orden no reconocida"<<endl;
            mtxScreen.unlock();
        }

        bzero(buffer,BUFFSIZE);
    }
    /*mtxScreen.lock();
    cout<<"servidor terminado"<<endl;
    mtxScreen.unlock();*/
};

// manejador que recive los mensajes de los clientes y los renvia a los demas (thread)
void clientHandler(int fd){
    int r, tam;
    string msg;
    char headBuffer[10], buffer[BUFFSIZE];
    struct PACKET recvPacket;
    bzero(buffer,BUFFSIZE);
    bzero(headBuffer,10);

    while (isconnected){
        if (recv (fd, headBuffer, USERNAMESIZE + COMMANDSIZE, 0) < 0){                        
            closeConection(fd);
            break;
        }        
        recvPacket.analizeHeader(headBuffer);
        //opciones del header        
        if (recvPacket.opt == "m"){
            //leer lo que queda del paquete y reenviar
            if (recv (fd, buffer, 5, 0) < 0){
                closeConection(fd);
                break;
            }
            recvPacket.analizeCordinates(buffer);
            msg = recvPacket.generate();
            
            for (int i=0 ; i<clientsFD.size(); ++i)
                send(clientsFD[i], msg.c_str(), msg.size(), 0);

        }
        else if (recvPacket.opt == "x"){

        }
        else if (recvPacket.opt == "s"){

        }
        else if (recvPacket.opt == "c"){ //chat

        }
        else if (recvPacket.opt == "e"){ //chat            
            closeConection(fd);
            return ;
        }
        else {
            // opcion no reconocida
        } 

        bzero(buffer,BUFFSIZE);
        bzero(headBuffer,10);
    }

}

int main(int argc, char **argv) {
    isconnected = true;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout<<"error al crear el socket"<<endl;
        return -1;
    }
    //struct sockaddr_in direc;
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {        
        cout<<"error al asignar puerto"<<endl;
        return -1;
    }
 
    if(listen(sockfd, 10) == -1) {        
        cout << "error en liste"<<endl;
        return -1;
    }
 
    /* initiate interrupt handler for IO controlling */
    thread (handler).detach();
 
    /* keep accepting connections */
    socklen_t sin_size = sizeof(serv_addr);;
    int newfd;
    while(isconnected) {
        //accept(servidor, (struct sockaddr*)&direc, &direcSize
        if((newfd = accept(sockfd, (struct sockaddr *)&serv_addr, &sin_size)) == -1) {
            continue;
        }        

        mtxScreen.lock();
        clientsFD.push_back(newfd);
        mtxScreen.unlock();

        thread (clientHandler,newfd).detach();        
    }
 
    return 0;
} 
