/*
 * pokemon_utils.h
 *
 *  Created on: 4 jun. 2020
 *      Author: utnso
 */



#ifndef POKEMON_UTILS_H_

#define POKEMON_UTILS_H_
#include "../../OurLibraries/UniversoPokemon/universo.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "team.h"
t_pokemon_team* get_pokemon(char* pokemon_name);
t_list* get_pokemones(t_config* config, int32_t index);
void mostrar_pokemon(t_pokemon_team* pokemon_actual, t_list* objetivo_global);
int32_t get_cantidad_pokemon(t_list* list_pokemones);
bool puede_capturar_pokemones(t_entrenador* entrenador);
t_list* sumarizar_pokemones(t_list* lista_pokemones_sin_sumarizar);
t_list* get_nombres_pokemon();
char* get_nombre_aleatorio(t_list* nombre_pokemones);
t_list* localized_to_pokemon_team(t_Localized mensaje_localized);
t_pokemon_team* get_pokemon_team(char* nombre, t_posicion posicion);
void liberar_elementos_lista_pokemon(t_list* lista);
void liberar_elementos_lista_entrenador(t_list* lista);
void liberar_elementos_lista_deadlock(t_list* lista);
void entrenador_destroyer(void* elem);
t_list* list_eliminar_int_repetidos(t_list* list);
void liberar_elementos_lista_respuesta(t_list* lista);
#endif /* POKEMON_UTILS_H_ */
