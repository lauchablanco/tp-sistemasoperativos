/*
 * universo.c
 *
 *  Created on: 2 may. 2020
 *      Author: utnso
 */


#include "universo.h"

int32_t universo_init(){
	return 0;
}


int32_t tamanio_pokemon (t_pokemon* pokemon){
	return (pokemon->size_Nombre + sizeof(pokemon->size_Nombre));
}

int32_t tamanio_new (t_New* nuevo_pokemon) {
	int32_t tam = 0;

	tam += nuevo_pokemon->pokemon.size_Nombre;

	tam += sizeof(nuevo_pokemon->pokemon.size_Nombre);

	tam += sizeof(nuevo_pokemon->posicion);

	tam += sizeof(nuevo_pokemon->cant);

	return tam;
}

int32_t tamanio_localized (t_Localized* localized){

	int32_t tamanio = 0;

	tamanio += tamanio_pokemon (&(localized->pokemon));
	tamanio += sizeof(localized->listaPosiciones->elements_count);
	if(localized->listaPosiciones->elements_count != 0){
		tamanio += sizeof(t_posicion) * localized->listaPosiciones->elements_count;
	}

	return tamanio;
}

int32_t tamanio_get (t_Get* get){
	return (tamanio_pokemon (&(get->pokemon)));
}

int32_t tamanio_appeared (t_Appeared* appeared){
	return (tamanio_pokemon (&(appeared->pokemon)) + sizeof(t_posicion));
}

int32_t tamanio_catch (t_Catch* catch){
	return (tamanio_pokemon (&(catch->pokemon)) + sizeof(t_posicion));
}

int32_t tamanio_caught (t_Caught* caught){
	return sizeof(t_Caught);
}





