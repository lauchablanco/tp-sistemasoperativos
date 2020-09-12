#ifndef GAME_CARD_H_
#define GAME_CARD_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/sockets.c"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/sockets.h"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/mensajes.c"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/mensajes.h"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.c"
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.h"
#include "/home/utnso/workspace/tp-2020-1c-5rona/Game-Card/src/funciones.h"

//Punteros globales:
t_log* logger_GC = NULL;

t_config* config_GC = NULL;

char* tiempo_reintento_con = NULL;
char* tiempo_reintento_op = NULL;
char* tiempo_retardo_op = NULL;
char* punto_de_montaje = NULL;
char* tam_bloque = NULL;
char* cant_bloques = NULL;
char* magic_number = NULL;
char* ip_broker = NULL;
char* puerto_broker = NULL;

t_dictionary* semaforos = NULL;

//Datos globales:
t_bitarray mapa_de_bloques;

sem_t diccionario;
sem_t bitmap;
sem_t envio_respuesta;

int32_t tiempo_reintento_conexion;
int32_t tiempo_reintento_operacion;
int32_t tiempo_retardo_operacion;
int32_t tamanio_bloque;
int32_t cantidad_bloques;
int32_t tam_punto_de_montaje;

int32_t id_proceso = 0;

//Prototipos de funciones:
void instalar_filesystem();
void recuperar_datos();
void crear_servidor_GC();
void responder_mensaje(int32_t socket, op_code codigo_operacion, int32_t id_mensaje);
void conexionBroker(int32_t *socket);
void hilo_suscriptor(op_code* code);
void suscribirse_broker();
void salir (const char* mensaje);
void inicializaciones_globales();
void liberar_memoria ();
int32_t reconectar(int32_t socket);


#endif
