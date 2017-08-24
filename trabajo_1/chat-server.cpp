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

#define BUFFERSIZE 1024
#define PORTNUMBER 5000

int main(){
    int cliente, servidor;
    bool finished = false;
    char buffer [BUFFERSIZE];

    struct sockaddr_in direc; //arpa
    socklen_t direcSize;

    if ((servidor = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout<<"Error al crear socket"<<endl;
        exit(1);
    }
    cout<<"socket del servidor creado correctamente"<<endl;

    direc.sin_family = AF_INET;
    direc.sin_addr.s_addr = htons(INADDR_ANY);
    direc.sin_port = htons(PORTNUMBER);

    if ((bind(servidor, (struct sockaddr*)&direc, sizeof(direc))) < 0){
        cout<<"error en bind"<<endl;
        exit(-1);
    }

    direcSize = sizeof(direc);
    cout<<"esperando clientes ..."<<endl;
    listen(servidor,1);

    while ((cliente = accept(servidor, (struct sockaddr*)&direc, &direcSize)) > 0){
        strcpy(buffer, "servidor conectado");
        cout<<buffer<<endl;
        send(cliente,&buffer,BUFFERSIZE,0);
        cout<<"conexion con el cliente lograda"<<endl;
        cout<<"escribir # para terminar conexion"<<endl;

        while (!finished){
            recv(cliente, &buffer, BUFFERSIZE, 0);
            cout<<"cliente: "<<buffer<<endl;
            if (*buffer == '#')
                finished = true;
            else{
                cout<<"servidor: "<<endl;
                cin.getline(buffer,BUFFERSIZE);
                send(cliente, &buffer, BUFFERSIZE, 0);
                if (*buffer == '#'){
                    finished = true;
                }
            }
        }
        cout<<endl<<"el servidor termino la conexion con "<<inet_ntoa(direc.sin_addr)<<endl;
        close(servidor); //cerrando socket
        cout<<"esperando por otro cliente"<<endl;
        finished = false;
    }
    close(cliente); //cerramos el socket
    return 0;
}
