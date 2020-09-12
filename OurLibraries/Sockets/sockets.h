/*
 * socketsUnificado.h
 *
 *  Created on: 29 abr. 2020
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netdb.h>
#include<sys/socket.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<signal.h>
#include<unistd.h>
#include<stdint.h>
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.h"


typedef enum
{
	SUSCRIPCION_NEW,
	SUSCRIPCION_APPEARED,
	SUSCRIPCION_CATCH,
	SUSCRIPCION_CAUGHT,
	SUSCRIPCION_GET,
	SUSCRIPCION_LOCALIZED,
	NEW_POKEMON,
	APPEARED_POKEMON,
	CATCH_POKEMON,
	CAUGHT_POKEMON,
	GET_POKEMON,
	LOCALIZED_POKEMON,
	ACK,
	HANDSHAKE,
} op_code;

typedef struct
{
	int32_t size;
	int32_t id_Mensaje;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


int32_t crear_conexion(char* ip, char* puerto);
void liberar_conexion(int32_t socket_cliente);

int32_t crear_socket_escucha(char *ip_servidor, char* puerto_servidor);
int32_t recibir_cliente(int32_t);

void* serializar_paquete_new (t_paquete*, int32_t*, t_New*);
t_New* deserializar_paquete_new (int32_t* socket_cliente);
void* serializar_paquete_appeared (t_paquete* paquete, int32_t* bytes, t_Appeared* appeared);
t_Appeared* deserializar_paquete_appeared (int32_t* socket_cliente);
void* serializar_paquete_catch (t_paquete* paquete, int32_t* bytes, t_Catch* catch);
t_Catch* deserializar_paquete_catch (int32_t* socket_cliente);
void* serializar_paquete_get (t_paquete* paquete, int32_t* bytes, t_Get* get);
t_Get* deserializar_paquete_get (int32_t* socket_cliente);
void* serializar_paquete_caught (t_paquete* paquete, int32_t* bytes, t_Caught* caught);
t_Caught* deserializar_paquete_caught (int32_t* socket_cliente);
void* serializar_paquete_localized (t_paquete* paquete, int32_t* bytes, t_Localized* localized);
t_Localized* deserializar_paquete_localized (int32_t* socket_cliente);

void enviar_handshake(int32_t id_proceso, int32_t socket_cliente);
void * serializar_handshake(t_paquete* paquete, int32_t* bytes);
void enviar_suscripcion(op_code operacion, int32_t socket_cliente);
void* serializar_paquete_suscripcion(t_paquete* paquete, int32_t* bytes);

void enviar_ACK(int32_t id_mensaje, int32_t socket_cliente);
void * serializar_ACK(t_paquete* paquete, int32_t* bytes);
int crear_servidor(int puerto);


#endif /* SOCKETS_H_ */
