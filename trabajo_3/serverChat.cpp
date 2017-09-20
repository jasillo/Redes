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
 
#define IP "127.0.0.1"
#define PORT 8080
 
#define BUFFSIZE 1024
#define ALIASLEN 32
#define OPTLEN 16

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
        message = getWord(&command);
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

vector<int> clientsFD;
 
int sockfd;
bool isconnected;

void handler(){
    char buffer[BUFFSIZE]; 
    
    while(cin.getline(buffer, BUFFSIZE)) {
        //analizar linea de comando dentro de packet
        string line = buffer;
        string option = getWord(&line);

        // ejecutar la opcion elegida
        if (option == "exit"){
            isconnected = false;
            close(sockfd);;
            break;
        }       
        else 
            cout<<"orden no reconocida"<<endl;

        bzero(buffer,BUFFSIZE);
    }
};

void clientHandler(int fd){
    int r, tam;
    string msg;
    char headBuffer[4], buffer[BUFFSIZE];
    struct PACKET recvPacket;

    while (isconnected){
        r = recv (fd, headBuffer, 4, 0);
        if (r < 0){
            cout<<"se perdio la coneccion"<<endl;            
            close(fd);
            break;
        }
        tam = atoi(headBuffer);
        msg = headBuffer;

        r = recv (fd, buffer, tam, 0);
        if (r < 0){
            cout<<"se perdio la coneccion"<<endl;
            isconnected = false;
            close(sockfd);
            break;
        }
        
        recvPacket.analizeInstruction(buffer);
        if (recvPacket.opt == "login"){
            cout<<"usuario conectado como "<<recvPacket.user<<endl;    
        }
        else if (recvPacket.opt == "send"){
              
        }else{
            cout<<recvPacket.opt<<endl;
        }

        bzero(buffer,BUFFSIZE);
        bzero(headBuffer,4);
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
    while(1) {
        //accept(servidor, (struct sockaddr*)&direc, &direcSize
        if((newfd = accept(sockfd, (struct sockaddr *)&serv_addr, &sin_size)) == -1) {
            cout<<"error al aceptar coneccion"<<endl;
            return -1;
        }
        
        if(clientsFD.size() == 10 ) {
            cout<<"conexiones llenas"<<endl;
            continue;
        }

        clientsFD.push_back(newfd);
        thread (clientHandler,newfd).detach();        
    }
 
    return 0;
} 
