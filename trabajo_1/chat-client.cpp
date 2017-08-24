//
// Created by jorge on 19/04/17.
//


#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main(){
    int servidor;
    int portNumber = 1500;
    int bufferSize = 1024;
    char *buffer = new char(bufferSize);
    bool finished = false;
    char* ip = (char*)"127.0.0.1";
    string msm;

    struct  sockaddr_in direc; //in.h

    if((servidor = socket(AF_INET, SOCK_STREAM, 0)) < 0){ //socket.h
        cout<<"error al crear el socket"<<endl;
        exit(0);
    }
    cout<<"socket creado"<<endl;
    cout<<"escriba # para acabar la conexion"<<endl;

    direc.sin_family = AF_INET;
    direc.sin_port = htons(portNumber);
    inet_pton(AF_INET, ip, &direc.sin_addr);

    if (connect(servidor,(struct sockaddr *)&direc, sizeof(direc)) == 0){
        cout<<"conexion con el servidor: "<<inet_ntoa(direc.sin_addr)<<endl;
    } else{
        cout<<"conexion fallo"<<endl;
    }


    
    recv(servidor, buffer, bufferSize, 0);

    cout<<"servidor: "<<buffer<<endl;
    cout<<"poner * al final de cada linea"<<endl;

    while (!finished){
        cout<<"cliente: "<<endl;
        cin.getline(buffer,bufferSize);
        send(servidor,buffer,bufferSize,0);
        if (*buffer == '#')
            finished = true;
        else{
            recv(servidor, buffer, bufferSize, 0);
            cout<<"servidor: "<<buffer<<endl;
            if (*buffer == '#')
                finished = true;
        }
    }
    cout<<"conexion terminada, cliente terminado"<<endl;
    close(servidor);
    delete[] buffer;
    return 0;
}
