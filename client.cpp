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
#define PORTNUMBER 41000

int main(){
    int servidor; //socket
    
    char buffer[BUFFERSIZE];
    bool finished = false;
    string ip = "162.168.110.120";

    struct  sockaddr_in direc; //in.h

    if((servidor = socket(AF_INET, SOCK_STREAM, 0)) < 0){ //socket.h
        cout<<"error al crear el socket"<<endl;
        exit(0);
    }
    cout<<"socket creado"<<endl;
    cout<<"escriba # para acabar la conexion"<<endl;

    direc.sin_family = AF_INET;
    direc.sin_port = htons(PORTNUMBER);
    inet_pton(AF_INET, ip.c_str(), &direc.sin_addr);

    if (connect(servidor,(struct sockaddr *)&direc, sizeof(direc)) == 0){
        cout<<"conexion con el servidor: "<<inet_ntoa(direc.sin_addr)<<endl;
    } else{
        cout<<"conexion fallo"<<endl;
    }
	
	cin.getline(buffer,BUFFERSIZE);
	send(servidor,&buffer,BUFFERSIZE,0);   

	recv(servidor, &buffer, BUFFERSIZE, 0);
    cout<<"servidor: "<<buffer<<endl;
    
    cout<<"conexion terminada, cliente terminado"<<endl;
    close(servidor);
    return 0;
}
