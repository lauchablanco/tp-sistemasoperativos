/*
 * socketsUnificado.c
 *
 *  Created on: 29 abr. 2020
 *      Author: utnso
 */

#include "sockets.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

struct sockaddr_in  direccionServidor;

/****************************************************************************/
/**********************************CONEXION**********************************/
/****************************************************************************/

int32_t crear_conexion(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int32_t socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		freeaddrinfo(server_info);
		return 0;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void liberar_conexion(int32_t socket_cliente){
	close(socket_cliente);
}

/////////////Server:

int32_t crear_socket_escucha(char *ip_servidor, char* puerto_servidor){
	int32_t socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_servidor, puerto_servidor, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int crear_servidor(int puerto){
    /*== creamos el socket ==*/
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = INADDR_ANY;
    direccionServidor.sin_port = htons(puerto);
    int _servidor = socket(AF_INET,SOCK_STREAM,0);
    /*== socket reusable multiples conexiones==*/
    uint32_t flag = 1;
    setsockopt(_servidor, SOL_SOCKET,SO_REUSEPORT,&flag,sizeof(flag));
    /*== inicializamos el socket ==*/
    if(bind(_servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0){
        perror("Fallo el binde0 del servidor");
        return 1;
    }

    listen(_servidor,SOMAXCONN);
    return _servidor;
}

int32_t recibir_cliente(int32_t socket_servidor){
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	return accept(socket_servidor, (struct sockaddr*) &dir_cliente, &tam_direccion);
	//       ^ accept crea un nuevo socket para el cliente

}



/*********************************************************************************/
/**********************************SERIALIZACION**********************************/
/*********************************************************************************/


void* serializar_paquete_new (t_paquete* paquete, int32_t* bytes, t_New* new){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(new->pokemon.size_Nombre), sizeof(new->pokemon.size_Nombre));
	desplazamiento+= sizeof(new->pokemon.size_Nombre);
	memcpy(stream + desplazamiento, new->pokemon.nombre, new->pokemon.size_Nombre);
	desplazamiento+= new->pokemon.size_Nombre;
	memcpy(stream + desplazamiento, &(new->posicion.X), sizeof(new->posicion.X));
	desplazamiento+= sizeof(new->posicion.X);
	memcpy(stream + desplazamiento, &(new->posicion.Y), sizeof(new->posicion.Y));
	desplazamiento+= sizeof(new->posicion.Y);
	memcpy(stream + desplazamiento, &(new->cant), sizeof(new->cant));

	return stream;
}

t_New* deserializar_paquete_new (int32_t* socket_cliente) {

	t_New* new = malloc (sizeof(t_New));

	recv (*socket_cliente, &(new->pokemon.size_Nombre), sizeof(new->pokemon.size_Nombre), MSG_WAITALL);

	new->pokemon.nombre = malloc (new->pokemon.size_Nombre);

	recv(*socket_cliente, new->pokemon.nombre, new->pokemon.size_Nombre, MSG_WAITALL);

	recv(*socket_cliente, &(new->posicion.X), sizeof(new->posicion.X), MSG_WAITALL);

	recv(*socket_cliente, &(new->posicion.Y), sizeof(new->posicion.Y), MSG_WAITALL);

	recv(*socket_cliente, &(new->cant), sizeof(new->cant), MSG_WAITALL);

	return new;
}

void* serializar_paquete_appeared (t_paquete* paquete, int32_t* bytes, t_Appeared* app){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(app->pokemon.size_Nombre), sizeof(app->pokemon.size_Nombre));
	desplazamiento+= sizeof(app->pokemon.size_Nombre);
	memcpy(stream + desplazamiento, app->pokemon.nombre, app->pokemon.size_Nombre);
	desplazamiento+= app->pokemon.size_Nombre;
	memcpy(stream + desplazamiento, &(app->posicion.X), sizeof(app->posicion.X));
	desplazamiento+= sizeof(app->posicion.X);
	memcpy(stream + desplazamiento, &(app->posicion.Y), sizeof(app->posicion.Y));

	return stream;
}

t_Appeared* deserializar_paquete_appeared (int32_t* socket_cliente) {

	t_Appeared* app = malloc (sizeof(t_Appeared));

	recv (*socket_cliente, &(app->pokemon.size_Nombre), sizeof(app->pokemon.size_Nombre), MSG_WAITALL);

	app->pokemon.nombre = malloc (app->pokemon.size_Nombre);

	recv(*socket_cliente, app->pokemon.nombre, app->pokemon.size_Nombre, MSG_WAITALL);

	recv(*socket_cliente, &(app->posicion.X), sizeof(app->posicion.X), MSG_WAITALL);

	recv(*socket_cliente, &(app->posicion.Y), sizeof(app->posicion.Y), MSG_WAITALL);

	return app;
}

void* serializar_paquete_catch (t_paquete* paquete, int32_t* bytes, t_Catch* catch){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(catch->pokemon.size_Nombre), sizeof(catch->pokemon.size_Nombre));
	desplazamiento+= sizeof(catch->pokemon.size_Nombre);
	memcpy(stream + desplazamiento, catch->pokemon.nombre, catch->pokemon.size_Nombre);
	desplazamiento+= catch->pokemon.size_Nombre;
	memcpy(stream + desplazamiento, &(catch->posicion.X), sizeof(catch->posicion.X));
	desplazamiento+= sizeof(catch->posicion.X);
	memcpy(stream + desplazamiento, &(catch->posicion.Y), sizeof(catch->posicion.Y));

	return stream;
}

t_Catch* deserializar_paquete_catch (int32_t* socket_cliente) {

	t_Catch* catch = malloc (sizeof(t_Catch));

	recv (*socket_cliente, &(catch->pokemon.size_Nombre), sizeof(catch->pokemon.size_Nombre), MSG_WAITALL);

	catch->pokemon.nombre = malloc (catch->pokemon.size_Nombre);

	recv(*socket_cliente, catch->pokemon.nombre, catch->pokemon.size_Nombre, MSG_WAITALL);

	recv(*socket_cliente, &(catch->posicion.X), sizeof(catch->posicion.X), MSG_WAITALL);

	recv(*socket_cliente, &(catch->posicion.Y), sizeof(catch->posicion.Y), MSG_WAITALL);

	return catch;
}

void* serializar_paquete_get (t_paquete* paquete, int32_t* bytes, t_Get* get){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);
	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(get->pokemon.size_Nombre), sizeof(get->pokemon.size_Nombre));
	desplazamiento+= sizeof(get->pokemon.size_Nombre);
	memcpy(stream + desplazamiento, get->pokemon.nombre, get->pokemon.size_Nombre);

	return stream;
}

t_Get* deserializar_paquete_get (int32_t* socket_cliente) {

	t_Get* get = malloc (sizeof(t_Get));

	recv (*socket_cliente, &(get->pokemon.size_Nombre), sizeof(get->pokemon.size_Nombre), MSG_WAITALL);

	get->pokemon.nombre = malloc (get->pokemon.size_Nombre);

	recv(*socket_cliente, get->pokemon.nombre, get->pokemon.size_Nombre, MSG_WAITALL);

	return get;
}

void* serializar_paquete_caught (t_paquete* paquete, int32_t* bytes, t_Caught* caught){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);
	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(caught->fueAtrapado), sizeof(caught->fueAtrapado));

	return stream;
}

t_Caught* deserializar_paquete_caught (int32_t* socket_cliente) {

	t_Caught* caught = malloc (sizeof(t_Caught));

	recv (*socket_cliente, &(caught->fueAtrapado), sizeof(caught->fueAtrapado), MSG_WAITALL);

	return caught;
}

void* serializar_paquete_localized (t_paquete* paquete, int32_t* bytes, t_Localized* localized){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje) + paquete->buffer->size;
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);
	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));
	desplazamiento+= sizeof(paquete->buffer->id_Mensaje);

	memcpy(stream + desplazamiento, &(localized->pokemon.size_Nombre), sizeof(localized->pokemon.size_Nombre));
	desplazamiento+= sizeof(localized->pokemon.size_Nombre);
	memcpy(stream + desplazamiento, localized->pokemon.nombre, localized->pokemon.size_Nombre);
	desplazamiento+= localized->pokemon.size_Nombre;

	memcpy(stream + desplazamiento, &(localized->listaPosiciones->elements_count), sizeof(localized->listaPosiciones->elements_count));
	desplazamiento+= sizeof(localized->listaPosiciones->elements_count);

	int i;
	t_posicion * posicion;
	for(i=0; i < localized->listaPosiciones->elements_count; i++){
		posicion = list_get(localized->listaPosiciones, i);

		memcpy(stream + desplazamiento, &(posicion->X), sizeof(posicion->X));
		desplazamiento+= sizeof(posicion->X);
		memcpy(stream + desplazamiento, &(posicion->Y), sizeof(posicion->Y));
		desplazamiento+= sizeof(posicion->Y);
	}

	return stream;
}

t_Localized* deserializar_paquete_localized (int32_t* socket_cliente) {

	t_Localized* localized = malloc (sizeof(t_Localized));

	recv (*socket_cliente, &(localized->pokemon.size_Nombre), sizeof(localized->pokemon.size_Nombre), MSG_WAITALL);

	localized->pokemon.nombre = malloc(localized->pokemon.size_Nombre);

	recv(*socket_cliente, localized->pokemon.nombre, localized->pokemon.size_Nombre, MSG_WAITALL);

	localized->listaPosiciones = list_create();
	int32_t cantARecibir;
	recv(*socket_cliente, &(cantARecibir), sizeof(cantARecibir), MSG_WAITALL);

	if (cantARecibir != 0) {
		t_posicion* posicion = malloc(sizeof(t_posicion));
		for(int i=0; i < cantARecibir; i++){

			recv(*socket_cliente, &(posicion->X), sizeof(posicion->X), MSG_WAITALL);
			recv(*socket_cliente, &(posicion->Y), sizeof(posicion->Y), MSG_WAITALL);
			list_add(localized->listaPosiciones, posicion);
		}
	}

	return localized;
}

void enviar_handshake(int32_t id_proceso, int32_t socket_cliente){
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = HANDSHAKE;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = sizeof(NULL);
	paquete->buffer->id_Mensaje = id_proceso;
	paquete->buffer->stream = NULL;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_handshake(paquete, &bytes_a_enviar);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

void * serializar_handshake(t_paquete* paquete, int32_t* bytes){
	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje);
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));

	return stream;
}

void enviar_suscripcion(op_code operacion, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = sizeof(NULL);
	paquete->buffer->id_Mensaje = 0;
	paquete->buffer->stream = NULL;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_suscripcion (paquete, &bytes_a_enviar);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

void* serializar_paquete_suscripcion(t_paquete* paquete, int32_t* bytes){

	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje);
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));

	return stream;
}

void enviar_ACK(int32_t id_mensaje, int32_t socket_cliente){
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = ACK;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = sizeof(NULL);
	paquete->buffer->id_Mensaje = id_mensaje;
	paquete->buffer->stream = NULL;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_ACK(paquete, &bytes_a_enviar);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

void * serializar_ACK(t_paquete* paquete, int32_t* bytes){
	*bytes = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + sizeof(paquete->buffer->id_Mensaje);
	void *stream = malloc(*bytes);
	int32_t desplazamiento = 0;

	memcpy(stream, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);
	memcpy(stream + desplazamiento, &(paquete->buffer->id_Mensaje), sizeof(paquete->buffer->id_Mensaje));

	return stream;
}
