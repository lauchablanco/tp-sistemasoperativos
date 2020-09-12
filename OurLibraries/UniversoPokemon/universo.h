/*
 * universo.h
 *
 *  Created on: 2 may. 2020
 *      Author: utnso
 */

#ifndef UNIVERSO_H_
#define UNIVERSO_H_

#include<commons/collections/list.h>
#include<stdint.h>


/****************************************/
/* UNIVERSO POKEMON*/
/****************************************/


typedef struct {
	int32_t X;
	int32_t Y;
} t_posicion;

typedef struct {
	int32_t size_Nombre;
	char * nombre;
} t_pokemon;
// con esta estructura ocupa maximo 26 bytes, minimo 19 bytes
// nombre mas corto: Mew/Muk (3 bytes); nombre mas largo: Feraligatr (10 bytes)

/* Lauti:
 * podríamos imaginar un caso que envíen laevoluciondepikachu, hay que contemplar casos que pueden enviar cualquier cosa
 * no digo que lo hagan pero no creo que podamos confirmar al 100% el tamaño del tipo de dato, hay que calcularlo
*/


typedef struct {
	t_list pokemones;/* aca iria un t_list * listaPokemones y cuando haces list_add le pasas
								como parametro el tipo t_pokemon*/
} t_objetivo;



/****************************************/
/* PAYLOADS DE MENSAJES*/
/****************************************/

typedef struct {
	t_pokemon pokemon;
	t_posicion posicion;
	int32_t cant;
} t_New;
// ocupa entre 35 y 42 bytes

typedef struct {
	t_pokemon pokemon;
	t_list * listaPosiciones;
} t_Localized;

typedef struct {
	t_pokemon pokemon;
} t_Get;

typedef struct {
	t_pokemon pokemon;
	t_posicion posicion;
} t_Appeared;

typedef struct {
	t_pokemon pokemon;
	t_posicion posicion;
} t_Catch;

typedef struct {
	int32_t fueAtrapado; // 1 cuando fue atrapado, 0 cuando no fue atrapado
} t_Caught;

int32_t universo_init();
int32_t tamanio_pokemon (t_pokemon* pokemon);
int32_t tamanio_new (t_New* new);
int32_t tamanio_localized (t_Localized* localized);
int32_t tamanio_get (t_Get* get);
int32_t tamanio_appeared (t_Appeared* appeared);
int32_t tamanio_catch (t_Catch* catch);
int32_t tamanio_caught (t_Caught* caught);

#endif /*UNIVERSO_H_*/
