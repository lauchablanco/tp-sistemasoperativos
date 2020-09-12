/*
 * pokemon_utlis.c
 *
 *  Created on: 4 jun. 2020
 *      Author: utnso
 */
#include "pokemon_utils.h"


bool puede_capturar_pokemones(t_entrenador* entrenador){
	return get_cantidad_pokemon(entrenador->pokemones) < get_cantidad_pokemon(entrenador->objetivo);
}


t_pokemon_team* get_pokemon(char* pokemon_name){
	t_pokemon_team* pokemon = malloc(sizeof(t_pokemon_team));

	pokemon->cantidad = 1;
	pokemon->nombre = pokemon_name;

	return pokemon;
}


t_list* get_pokemones(t_config* config, int32_t index){
	t_list* pokemones_list = list_create();

	char** pokemon_entrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");

	char** pokemones = get_array_by_index(pokemon_entrenadores, index);

	int i = 0;

	while(pokemones != NULL && pokemones[i] != NULL){
		t_pokemon_team* pokemon = get_pokemon(pokemones[i]);
		list_add_in_index(pokemones_list, i, pokemon);
		i++;
	}

	//liberar_strings(pokemones);
	liberar_strings(pokemon_entrenadores);

	return pokemones_list;
}

void mostrar_pokemon(t_pokemon_team* pokemon_actual, t_list* objetivo_global){
//	printf("| 	nombre: %s, cantidad: %d\n",
//			pokemon_actual->nombre,
//			pokemon_actual->cantidad );
}

int32_t get_cantidad_pokemon(t_list* list_pokemones){

	int32_t total_pokemon = 0 ;

	int32_t _sumar_pokemon(void* element){
		total_pokemon += ((t_pokemon_team*)element)->cantidad;
		return total_pokemon;
	}

	list_iterate(list_pokemones, (void*)_sumar_pokemon);

	return total_pokemon;
}


t_list* sumarizar_pokemones(t_list* lista_pokemones_sin_sumarizar){

	t_list* lista_sumarizada = list_create();
	for(int32_t i = 0; i < lista_pokemones_sin_sumarizar->elements_count; i++){
		t_pokemon_team* pokemon = list_get(lista_pokemones_sin_sumarizar, i);

		t_pokemon_team* clone = malloc(sizeof(t_pokemon_team));
		clone->cantidad = pokemon->cantidad;
		clone->nombre = pokemon->nombre;
		clone->posicion = pokemon->posicion;

		int32_t pokemon_encontrado = 0;

		if(lista_sumarizada->elements_count == 0){

			list_add(lista_sumarizada, clone);
		} else {

			for(int32_t j = 0; j < lista_sumarizada->elements_count; j++){
				t_pokemon_team* pokemon_sumarizado = list_get(lista_sumarizada, j);

				if(string_equals_ignore_case(pokemon_sumarizado->nombre, pokemon->nombre)){


					pokemon_sumarizado->cantidad++;
					pokemon_encontrado = 1;

				}
			}

			if(!pokemon_encontrado){
				list_add(lista_sumarizada, clone);
			}
		}
	}

	//liberar_elementos_lista_pokemon(lista_pokemones_sin_sumarizar);

	return lista_sumarizada;

}

t_list* get_nombres_pokemon(){
	t_list* nombre_pokemones = list_create();

	list_add(nombre_pokemones, "Charmander");
	list_add(nombre_pokemones, "Pikachu");
	list_add(nombre_pokemones, "Squirtle");
	list_add(nombre_pokemones, "Pidgey");
	list_add(nombre_pokemones, "Rattata");
	list_add(nombre_pokemones, "Bulbasaur");

	return nombre_pokemones;

}

char* get_nombre_aleatorio(t_list* nombre_pokemones){
	int i =  (rand() % (nombre_pokemones->elements_count - 1));
	return list_get(nombre_pokemones, i);
}

t_list* localized_to_pokemon_team(t_Localized mensaje_localized){
	t_list* pokemones = list_create();
	for(int i = 0; i < mensaje_localized.listaPosiciones->elements_count; i++){
		t_pokemon_team* pokemon = malloc(sizeof(t_pokemon_team));
		pokemon->cantidad = 1;
		pokemon->nombre = mensaje_localized.pokemon.nombre;
		pokemon->posicion = *(t_posicion*)(list_get(mensaje_localized.listaPosiciones, i));

		list_add(pokemones, pokemon);
	}

	return pokemones;
}

t_pokemon_team* get_pokemon_team(char* nombre, t_posicion posicion){
	t_pokemon_team* pokemon_team = malloc(sizeof(t_pokemon_team));
	pokemon_team->cantidad = 1;
	pokemon_team->nombre = nombre;
	pokemon_team->posicion = posicion;

	return pokemon_team;
}



void respuesta_destroyer(void* elem){
	t_respuesta* respuesta = (t_respuesta*) elem;
	free(respuesta);
}

void liberar_elementos_lista_respuesta(t_list* lista){
	if(lista->elements_count > 0){
		list_clean_and_destroy_elements(lista, respuesta_destroyer);
	}

	free(lista);
}



void pokemon_destroyer(void* elem){
	t_pokemon_team* pokemon = (t_pokemon_team*) elem;
	free(pokemon->nombre);
	free(pokemon);
}

void liberar_elementos_lista_pokemon(t_list* lista){
	if(lista->elements_count > 0){
		list_clean_and_destroy_elements(lista, pokemon_destroyer);
	}
	list_destroy(lista);
}


void entrenador_destroyer(void* elem){
	t_entrenador* entrenador = (t_entrenador*) elem;
	liberar_elementos_lista_pokemon(entrenador->objetivo);
	liberar_elementos_lista_pokemon(entrenador->pokemones);
	if(entrenador->pokemon_destino != NULL){
		pokemon_destroyer(entrenador->pokemon_destino);
	}
	sem_destroy(entrenador->semaforo);
	free(entrenador);
}

void liberar_elementos_lista_entrenador(t_list* lista){
	if(lista->elements_count > 0){
		list_clean_and_destroy_elements(lista, entrenador_destroyer);
	}

	list_destroy(lista);
}



void deadlock_destroyer(void* elem){
	int* proceso_involucrado = (int*) elem;
	free(proceso_involucrado);
}

void liberar_elementos_deadlock(t_list* lista){
	list_clean_and_destroy_elements(lista, deadlock_destroyer);
	free(lista);
}


void deadlock_list_destroyer(void* elem){
	t_list* procesos_involucrados = (t_list*) elem;
	liberar_elementos_deadlock(procesos_involucrados);
}

void liberar_elementos_lista_deadlock(t_list* lista){
	list_clean_and_destroy_elements(lista, deadlock_list_destroyer);
	free(lista);
}


bool int_in_list(int elem, t_list* list){

	for(int i = 0; i < list->elements_count; i++){
		int* e = list_get(list, i);

		if(elem == *e) return true;

	}

	return false;
}

t_list* list_eliminar_int_repetidos(t_list* list){
	t_list* new_list = list_create();

	for(int i = 0; i < list->elements_count; i++){
		int* elem = list_get(list, i);

		if(!int_in_list(*elem, new_list)){
			list_add(new_list, elem);
		}

	}

	return new_list;
}

