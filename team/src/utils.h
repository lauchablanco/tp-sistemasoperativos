/*
 * utils.h
 *
 *  Created on: 4 jun. 2020
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include "team.h"

char** get_array_by_index(char** array_from_config, int32_t index);
int32_t array_length(char** value_array);
t_config* get_config(char* name);
int32_t get_algoritmo_code(char* algoritmo);
t_algoritmo get_algoritmo(t_config* config);
t_list* get_objetivos(t_config* config, int32_t index);
t_posicion get_posicion(t_config* config, int32_t index);
t_entrenador* get_entrenador(t_config* config, int32_t index);
t_list* get_entrenadores(t_config* config, int32_t cantidadEntrenadores);
int32_t get_distancia_entre_puntos(t_posicion pos1, t_posicion pos2);
t_list* get_objetivo_global(t_list* entrenadores);
t_list* get_pokemones_capturados_sumarizados_global(t_list* entrenadores);
t_list* get_pokemones_capturados_sumarizados_entrenador(t_entrenador* entrenador);
t_entrenador* get_entrenador_planificable_mas_cercano(t_list* entrenadores, t_posicion posicion_pokemon);
t_list* filtrar_localized_repetidos(t_list* mensajes_localized);
t_list* filtrar_localized_objetivo_global(t_list* mensajes_localized, t_list* objetivo_global);
t_respuesta* get_respuesta(int32_t id, t_list* respuestas);
int get_cantidad_by_nombre_pokemon(char* pokemon, t_list* pokemones);
bool esta_en_objetivos_globales(char* pokemon, t_list* objetivo_global);
bool fue_recibido(char* pokemon, t_list* pokemones_recibidos);
bool puedo_capturar(char* pokemon, t_list* entrenadores, t_list* objetivo_global);
bool appeared_valido(t_Appeared* mensaje, t_list* entrenadores, t_list* objetivo_global);
bool localized_valido(t_Localized* mensaje, int id, t_list* gets_recibidos, t_list* pokemones_recibidos, t_list* objetivo_global);
int32_t conexion_broker();
bool cumplio_objetivo(t_entrenador* entrenador);
t_posicion avanzar(t_posicion posicion, int32_t posX, int32_t posY);
t_pokemon_team* get_pokemon_necesario_mas_cercano(t_list* pokemones_ubicados, t_posicion posicion_entrenador);
void show_semaforo(sem_t* semaforo);
t_pokemon_team* get_pokemon_by_nombre(char* nombre_pokemon, t_list* pokemones);
t_list* pokemones_de_mas(t_entrenador* entrenador);
t_list* objetivos_pendientes(t_entrenador* entrenador);
t_pokemon_team* pokemon_que_sirve(t_entrenador* e1, t_entrenador* e2);
t_pokemon_team* remove_pokemon_by_nombre(char* nombre_pokemon, t_list* pokemones);
t_pokemon_team* pokemon_para_intercambio(t_entrenador* entrenador, t_pokemon_team* pokemon);
int32_t reconectar(int32_t socket);
t_list* sumarizar_pokemones(t_list* pokemones_sin_sumarizar);
t_pokemon_team* get_pokemon(char* pokemon_name);
t_list* get_pokemones(t_config* config, int32_t index);
bool puede_capturar_pokemones(t_entrenador* entrenador);
t_pokemon_team* pokemon_que_sirve_intercambio(t_entrenador* e1, t_entrenador* e2);
#endif /* UTILS_H_ */
