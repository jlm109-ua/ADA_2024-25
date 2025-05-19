/*
Exemple: iteració sobre els 8 possibles moviments en el problema del laberint
*/
#include<map>
#include<tuple>
#include<iostream>
using namespace std;

// Tipus enumerat amb les 8 possibles direccions
enum Step { N, NE, E, SE, S, SW, W, NW };

int main(){

	/* 
	Mapa: de direcció a tupla amb increments de fila i columna 
	*/
	map<Step,tuple<int,int>> steps_inc;
	steps_inc[N]=make_tuple(-1,0);
	steps_inc[NE]=make_tuple(-1,1);
	steps_inc[E]=make_tuple(0,1);
	steps_inc[SE]=make_tuple(1,1);
	steps_inc[S]=make_tuple(1,0);
	steps_inc[SW]=make_tuple(1,-1);
	steps_inc[W]=make_tuple(0,-1);
	steps_inc[NW]=make_tuple(-1,-1);
	
	/*
	Iteració sobre els elements del mapa
	*/
 	for ( auto it=steps_inc.begin(); it!=steps_inc.end(); ++it){

 	     //Direcció de moviment
 	     Step move=it->first;
	     
	     //Increment de fila i de columna
	     int incx, incy; 	     
	     tie(incx,incy)=it->second;
	     
	     cout << move << ": ("<< incx << "," << incy << ")" << endl; 
       }

   return 0;
}
