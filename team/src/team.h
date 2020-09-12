/*
 * team.h
 *
 *  Created on: 18 abr. 2020
 *      Author: utnso
 */

#ifndef TEAM_H_
#define TEAM_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../../OurLibraries/Sockets/sockets.h"
#include "../../OurLibraries/UniversoPokemon/universo.h"
#include "../../OurLibraries/Sockets/mensajes.h"
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>


typedef enum {
	RR    = 1,
	SJFSD = 2,
	SJFCD = 3,
	FIFO  = 4
} algoritmo_code;

typedef enum {
	NEW     = 1,
	READY   = 2,
	BLOCKED = 3,
	EXEC 	= 4,
	EXIT 	= 5
} estado_code;

typedef struct {
	int32_t 	algoritmo_code;
	char* 	algoritmo_string;
	int32_t 	quantum;
	int32_t 	retardo;
	double alpha;
}t_algoritmo;

typedef struct {
	int32_t id_respuesta;
	int32_t id_entrenador;

} t_respuesta;

typedef struct {
	void* mensaje;
	t_respuesta* respuesta;
}t_args_mensajes;


typedef struct {
	char * 	 	nombre;
	int32_t 	cantidad;
	t_posicion posicion;
	bool planificable;
}t_pokemon_team;

typedef struct {
	t_posicion posicion;
	t_pokemon_team* pokemon_destino;
	t_list* 	pokemones;
	t_list* 	objetivo;
	estado_code estado;
	int32_t id;
	bool ocupado;
	sem_t* semaforo;
	int estimacion_anterior;
	int cantidad_ejecutada;
	int estimacion;
	int ciclos;
} t_entrenador;

typedef struct {
	t_entrenador* entrenador;
	sem_t semaforo_entrenador;
}t_args_entrenador;

typedef struct {
	t_list* procesos_involucrados;
}t_deadlock;


t_log* logger;
t_config* config;


//Datos traidos por config
char* IP_BROKER;
char* PUERTO_BROKER;
int32_t PROCESS_ID;
t_list* objetivo_global;
t_list* entrenadores;
t_algoritmo algoritmo;
int TIEMPO_RECONEXION;
pthread_mutex_t mutex_pokemones_ubicados;
pthread_mutex_t mutex_list_entrenadores;

void hilo_suscriptor_appeared(op_code *code);
void hilo_suscriptor_caught(op_code *code);
void hilo_suscriptor_localized(op_code *code);

void recibidor_mensajes_appeared(t_Appeared* args);
bool estan_unidos(t_entrenador* nodo_actual, t_entrenador* nodo_siguiente);
//void recorrer_fila_DL(int fila, int** matriz);
void liberar_strings(char** cadenas);

#endif /* TEAM_H_ */
