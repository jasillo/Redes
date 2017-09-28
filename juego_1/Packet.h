#pragma once

#ifndef _PACKET_H_
#define _PACKET_H_

#include <string>
#include <iostream>
#include <algorithm>


#define USERNAMESIZE 2
#define COMMANDSIZE 1
#define GROUNDSIZE 20

using namespace std;

/********************************
* funciones de manejo de strings
*********************************/
// leer un palabra de un string, luego borra esa palabra
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

//elimina los espacios delante de un string
string getString(string *s){
    int pos;
    //eliminar espacios en blanco al inicio
    for (pos=0; pos < s->size() && s->at(pos) == ' '; ++pos){};
    *s = s->substr(pos);
    if (s->empty())
        return "";    
    
    return *s;
};

// construir strings de tamaño fijo con ints
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
int directions[] = {0,1, 1,1, 1,0, 1,-1, 0,-1, -1,-1, -1,0, -1,1 ,0,0};

struct Ground
{	
	vector<string> players;
	vector<int> x, y; //coordenadas de los jugadores
	
    vector<int> shotX, shotY, direX, direY;
    vector<clock_t> startTime;

	Ground(){		
	};

	bool setPosition(string player, int newX, int newY){
		//no se tocan los bordes -> 0 y GROUNDSIZE
		if ( newX<=0 || newX>=GROUNDSIZE || newY<=0 || newY>=GROUNDSIZE )
			return false;

        //detectar colisiones 
        int j;       
        for (int i=0; i< players.size(); ++i){
            if (x[i] == newX && y[i]== newY)
                return false;
            if (players[i] == player)
                j=i;
        }
		
		x[j] = newX;
		y[j] = newY;
		return true;		
	};

    bool setPositionNewPlayer(string player, int newX, int newY){        
        players.push_back(player);
        x.push_back(newX);
        y.push_back(newY);
        return true;
    }
};

struct PACKET { 
    string opt; //opcion del comando: send, login, logout
    string user; // nombre del usuario
    string message;
    int direc;

    int corX, corY; // coordenadas origen
    int dirX, dirY; // direccion de movimiento o disparo

    PACKET(){        
        clear();
    };

    void analizeCordinates(string command){    	
    	string s = command.substr(0,2);
    	corX = atoi(s.c_str());
    	s = command.substr(2,2);
    	corY = atoi(s.c_str());
    	s = command.substr(4,1);
    	direc = atoi(s.c_str());
    	
    	dirX = directions[direc*2];
    	dirY = directions[direc*2+1];
    };

    // analiza un commando para optener los parametros
    void analizeBody(string command){
        //se analiza el resto del mensaje (cordenadas o cadena)
    	
    };

    void analizeHeader(string command){    	
    	user = command.substr(0, USERNAMESIZE);
    	opt = command.substr(USERNAMESIZE, COMMANDSIZE);
    };

    void clear(){
        
    };
    string generate(){
    	string r;
	    if (opt == "c"){ //chat
	        r = user + opt ;
	        r += fixedLength(message.size(),4);
	        r += message;	        
        }
        else if (opt == "m" || opt == "s" || opt == "l"){
        	r = user + opt ;
        	r += fixedLength(corX,2);
        	r += fixedLength(corY,2);
        	r += fixedLength(direc,1);
        }
        else if (opt == "e"){
        	r = user + opt ;        	
        }        
        ///las demas oopciones
        return r;
    };    
};

#endif /* _PACKET_H_ */