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
 
#define SERVERIP "127.0.0.1"
#define SERVERPORT 8080 
#define BUFFSIZE 1024

using namespace std;

/********************************
* funciones de manejo de strings
*********************************/
string getWord(string *s){
    int pos;
    //eliminar espacios en blanco al inicio
    for (pos=0; pos < s->size() && s->at(pos) == ' '; ++pos){};
    *s = s->substr(pos);
    if (s->empty())
        return "";
    //obtencion de la primera palabra
    for (pos=0; pos < s->size() && s->at(pos) != ' '; ++pos){};
    string word = s->substr(0,pos);
    *s = s->substr(pos);
    return word;
};

string getString(string *s){
    int pos;
    //eliminar espacios en blanco al inicio
    for (pos=0; pos < s->size() && s->at(pos) == ' '; ++pos){};
    *s = s->substr(pos);
    if (s->empty())
        return "";    
    
    return *s;
};

std::string fixedLength(int value, int digits = 3) {
    unsigned int uvalue = value;
    if (value < 0) {
        uvalue = -uvalue;
    }
    std::string result;
    while (digits-- > 0) {
        result += ('0' + uvalue % 10);
        uvalue /= 10;
    }
    if (value < 0) {
        result += '-';
    }
    reverse(result.begin(), result.end());
    return result;
}
/***********************************************/


struct PACKET { 
    string opt; //opcion del comando: send, login, logout
    string user; // nombre del usuario
    string message;        

    PACKET(){        
        clear();
    };

    // analiza un commando para optener los parametros
    void analizeInstruction(string command){
        opt = getWord(&command);
        user = getWord(&command);
        message = getString(&command);
    };

    void clear(){
        opt.clear();
        user = "Anonymous";
        message.clear();
    };
    string generate(){
        string r = opt + " " + user + " " + message;
        r = fixedLength(r.size(),4) + r;
        
        return r;
    };    
};

int sockfd;
bool isconnected;
string user;

void receiver(){
    int r , tam;
    char buffer[BUFFSIZE], headBuffer[4];
    bzero(buffer,BUFFSIZE);
    bzero(headBuffer,4);
    struct PACKET recvPacket;

    while (isconnected){
        //recivimos tamaño del mensaje
        r = recv (sockfd, headBuffer, 4, 0);
        if (r < 0){
            cout<<"se perdio la coneccion"<<endl;
            isconnected = false;
            close(sockfd);
            break;
        }
        tam = atoi(headBuffer);

        r = recv (sockfd, buffer, tam, 0);
        if (r < 0){
            cout<<"se perdio la coneccion"<<endl;
            isconnected = false;
            close(sockfd);
            break;
        }
        
        recvPacket.analizeInstruction(buffer);
        if (recvPacket.opt == "send"){
            cout<<"["<<recvPacket.user<<"]: "<<recvPacket.message<<endl;    
        }
        else if (recvPacket.opt == "login"){
            cout<<"usuario nuevo se conecto como "<<recvPacket.user<<endl;   
        }
        else if (recvPacket.opt == "exit"){
            cout<<"usuario "<<recvPacket.user<<" se desconecto"<<endl;   
        }
        else{
            cout<<recvPacket.opt<<endl;
        }

        bzero(buffer,BUFFSIZE);
        bzero(headBuffer,4);
    }
};
 
void sendtoall(string msg) {
        
    if(!isconnected) {
        cout<<"no estas conectado"<<endl;
        return;
    } 
    
    struct PACKET sendPacket;
    sendPacket.opt = "send";
    sendPacket.user = user;
    sendPacket.message = msg;
    string sendMsg = sendPacket.generate(); 
    int s = send(sockfd, sendMsg.c_str(), sendMsg.size(),0);
    
    if (s < 0)
        cout<<"error al enviar mensaje"<<endl;
}



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
    user = u;
    isconnected = true;
    thread (receiver).detach();

    struct PACKET loginPacket;
    loginPacket.opt = "login";
    loginPacket.user = user;
    loginPacket.message = "";
    string loginMsg = loginPacket.generate(); 
    int s = send(sockfd, loginMsg.c_str(), loginMsg.size(),0);  
}

void logout() {
    
    int sent;
    struct PACKET exitPacket;
    
    if(!isconnected) {
        cout<<"ya estas desconectado"<<endl;
        return;
    }
    isconnected = false;

    exitPacket.opt = "exit";
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
    
    while(cin.getline(buffer, BUFFSIZE)) {
        //analizar linea de comando dentro de packet
        string line = buffer;
        string option = getWord(&line);

        // ejecutar la opcion elegida
        if (option == "exit"){
            logout();
            break;
        }       
        else if(option == "login") {            
            string u = getWord(&line);
            login(u);
        }               
        else if(option == "send") {                            
            sendtoall(line);
        }        
        else 
            cout<<"orden no reconocida"<<endl;

        bzero(buffer,BUFFSIZE);
    }
    return 0;
}
 

 

 
