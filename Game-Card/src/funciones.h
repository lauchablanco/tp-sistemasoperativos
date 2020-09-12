#ifndef SRC_FUNCIONES_H_
#define SRC_FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.h"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/mensajes.h"

//Punteros globales:
extern t_log* logger_GC;

extern char* punto_de_montaje;

extern t_dictionary* semaforos;

//Datos globales:

extern t_bitarray mapa_de_bloques;

extern sem_t diccionario;
extern sem_t bitmap;
extern sem_t envio_respuesta;

extern int32_t tiempo_reintento_operacion;
extern int32_t tiempo_retardo_operacion;
extern int32_t tamanio_bloque;
extern int32_t cantidad_bloques;
extern int32_t tam_punto_de_montaje;
extern char* ip_broker;
extern char* puerto_broker;


// Funciones que no requieren liberar memoria:
void funcion_new_pokemon(void* new_y_id); //stream con el new e id
void funcion_catch_pokemon(void* catch_y_id);
void funcion_get_pokemon(void* get_y_id);

int32_t existe (char* nombre_pokemon);
void crear_metadata_new (t_New* new, char* ruta);
off_t asignar_bloque_libre(void);
char get_open (char* ruta_metadata);
void set_open (char* ruta_metadata, char valor);
FILE* abrir_para (char* ruta, char* modo);
void actualizar_bitmap (void);
int32_t cantidad_de_bloques (char* vector);
void liberar_bloques (char** strings_bloques, int32_t cantidad, char* ruta_metadata, char* pokemon);
char* apuntar_a_posicion (char* archivo_cargado, t_posicion posicion);
char* agregar_cantidad(char* archivo_cargado, char* apuntador, int32_t cantidad_nueva, t_posicion posicion);
char* agregar_nueva_posicion(char* archivo_cargado, t_posicion posicion_nueva, int32_t cantidad_nueva);
void volcar_archivo_cargado(char* archivo_cargado, char* ruta_metadata, char* pokemon);
void metadata_agregar_bloque (char* ruta_metadata, char* pokemon, char* bloque);
void metadata_actualizar_size (char* ruta_metadata, char* pokemon, int32_t size_nuevo);
void metadata_volcar (char* ruta_metadata, char* pokemon, char* archivo_cargado);
void liberar_strings(char** cadenas);
char* quitar_pokemon (char* archivo_cargado, char* apuntador, t_posicion posicion);

extern void salir (const char* mensaje);

void enviar_appeared (char* pokemon, char* x, char* y, char* mensaje_id);
void enviar_caught (char* id_mensaje_correlativo, char * fueAtrapado);
void enviar_localized (t_list* posiciones, t_pokemon pokemon, int32_t id_mensaje_correlativo);

// Funciones que requieren liberar memoria:
char* ruta_metadata_pokemon_teorica (t_pokemon pokemon);
char* ruta_carpeta_pokemon_teorica (t_pokemon pokemon);
FILE* abrir_para (char* ruta, char* modo);
char* copiar_stream(FILE* archivo_lectura, char* ruta);
char* metadata_copiar_linea_bloques(FILE* metadata, char* ruta_metadata);
char* traer_bloques(char** bloques, int32_t cantidad);
char* posicion_a_string (t_posicion posicion);
char* metadata_traer (char* ruta_metadata, char* pokemon, int32_t* tam_alojamiento);
char* copiar_stream_con_tamanio(FILE* archivo_lectura, char* ruta, int32_t* tamanio_copiado);


#endif
