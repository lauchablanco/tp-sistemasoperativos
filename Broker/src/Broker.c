/*
 ============================================================================
 Name        : Broker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Broker.h"
#include "Particiones.h"
#include "../../OurLibraries/Sockets/mensajes.h"

int32_t id_mensaje_global = 0;
int reader = 0;

int32_t main(void) {
	iniciarBroker();

	signal(SIGUSR1, rutina);

	int32_t socketEscucha = crear_servidor(atoi(PUERTO_BROKER));//crear_socket_escucha(IP_BROKER, PUERTO_BROKER);
	log_info(logger, "Creado socket de escucha");

	while(socketEscucha != -1){
		int32_t socket_cliente = (int32_t)recibir_cliente(socketEscucha);

		if(socket_cliente != -1){

			int32_t tamanio_estructura = 0;
			int32_t id_mensaje=0;
			int32_t operacion=0;
			pthread_t hilo;
			int32_t id_proceso =0;
			t_estructura_hilo_mensaje estructura_mensaje;
			t_estructura_hilo_suscriptor estructura_suscriptor;

			//HANDSHAKE
			if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == HANDSHAKE){
					recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket_cliente, &id_proceso, sizeof(int32_t), MSG_WAITALL);
					log_info(logger, "Se conecto el proceso %d", id_proceso);
					//ACK DEL HANDSHAKE
					enviar_ACK(0, socket_cliente);
					//ESPERA EL MENSAJE
					if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						switch(operacion){
						case SUSCRIPCION_NEW:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_NEW;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionNew, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion New\n");
							pthread_detach(hilo);
						break;
						case SUSCRIPCION_APPEARED:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_APPEARED;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionAppeared, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion Appeared\n");
							pthread_detach(hilo);
						break;
						case SUSCRIPCION_GET:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_GET;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionGet, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion Get\n");
							pthread_detach(hilo);
						break;
						case SUSCRIPCION_LOCALIZED:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_LOCALIZED;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionLocalized, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion Localized\n");
							pthread_detach(hilo);
						break;
						case SUSCRIPCION_CATCH:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_CATCH;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionCatch, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion Catch\n");
							pthread_detach(hilo);
						break;
						case SUSCRIPCION_CAUGHT:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_list_suscriptores);
							estructura_suscriptor.socket_cliente = socket_cliente;
							estructura_suscriptor.operacion = SUSCRIPCION_CAUGHT;
							estructura_suscriptor.id_proceso = id_proceso;
							if (pthread_create(&hilo, NULL, (void*)manejoSuscripcionCaught, &estructura_suscriptor) == 0){
							}else printf("Fallo al crear el hilo que maneja la suscripcion Caught\n");
							pthread_detach(hilo);
						break;
						case NEW_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje New\n");
							pthread_detach(hilo);
							break;
						case APPEARED_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje Appeared\n");
							pthread_detach(hilo);
							break;
						case GET_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje Get\n");
							pthread_detach(hilo);
							break;
						case LOCALIZED_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje Localized\n");
							pthread_detach(hilo);
							break;
						case CATCH_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje Catch\n");
							pthread_detach(hilo);
							break;
						case CAUGHT_POKEMON:
							recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
							pthread_mutex_lock(&mutex_estructura_mensajes);
							estructura_mensaje.socket_cliente = socket_cliente;
							estructura_mensaje.id_mensaje = id_mensaje;
							estructura_mensaje.id_proceso = id_proceso;
							estructura_mensaje.operacion = operacion;
							if (pthread_create(&hilo, NULL, (void*)manejoMensaje, &estructura_mensaje) == 0){
							}else printf("Fallo al crear el hilo que maneja el mensaje Caught\n");
							pthread_detach(hilo);
							break;
							}
						} else printf("Fallo al recibir codigo de operacion = -1\n");
				}else printf("El proceso no se identifico \n");
			}
			else printf("Fallo al recibir codigo de operacion = -1\n");
		}else {
			printf("Fallo al recibir/aceptar al cliente\n");
			sleep(5);
			close(socketEscucha);
			socketEscucha = crear_socket_escucha(IP_BROKER, PUERTO_BROKER);
		}
	sleep(0.25);
	}
	if(socketEscucha == -1){
		printf("Fallo al crear socket de escucha = -1\n");
		sleep(2);
	}

	return EXIT_SUCCESS;

}

int32_t get_id(){
	pthread_mutex_lock(&mutex_id_mensaje);
	++id_mensaje_global;
	pthread_mutex_unlock(&mutex_id_mensaje);
	return id_mensaje_global;
}

void manejoSuscripcionNew(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor= NULL;
	info_mensaje * mensaje= NULL;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "NEW");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerNew);
		sem_wait(&nuevoMensajeNew);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}

void manejoSuscripcionAppeared(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor;
	info_mensaje * mensaje;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "APPEARED");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerApp);
		sem_wait(&nuevoMensajeApp);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}

void manejoSuscripcionGet(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor;
	info_mensaje * mensaje;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "GET");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerGet);
		sem_wait(&nuevoMensajeGet);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}
void manejoSuscripcionLocalized(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor;
	info_mensaje * mensaje;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "LOCALIZED");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerLoc);
		sem_wait(&nuevoMensajeLoc);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}

void manejoSuscripcionCatch(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor;
	info_mensaje * mensaje;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "CATCH");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerCatch);
		sem_wait(&nuevoMensajeCatch);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}

void manejoSuscripcionCaught(t_estructura_hilo_suscriptor * estructura_suscriptor){
	int32_t socket_cliente = estructura_suscriptor->socket_cliente;
	int32_t id_proceso = estructura_suscriptor->id_proceso;
	int32_t suscripcion = estructura_suscriptor->operacion;

	t_suscriptor * suscriptor;
	info_mensaje * mensaje;
	t_list * mensajesAEnviar = NULL;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje;
	int32_t operacion;
	bool fin = false;
	int32_t id_msg_log;

	if(procesoSuscriptoACola(suscripcion, id_proceso)){
		suscriptor = obtenerSuscriptor(id_proceso, suscripcion);
	} else{
		suscriptor = malloc(sizeof(t_suscriptor));
		suscriptor->id = id_proceso;
		suscriptor->op_code = suscripcion;
		list_add(list_suscriptores, suscriptor);

	}
	pthread_mutex_unlock(&mutex_list_suscriptores);

	char cola[9];
	strcpy(cola, "CAUGHT");
	log_info(logger, "Proceso suscripto a cola %s", cola);
	enviar_ACK(0, socket_cliente);

	while(fin == false){
		sem_wait(&primerCaught);
		sem_wait(&nuevoMensajeCaught);

		reader++;
		if(reader == 1){
			sem_wait(&writer);
		}

		pthread_mutex_lock(&mutex_guardar_en_memoria);
		mensajesAEnviar = getMensajesAEnviar(suscripcion, id_proceso);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);

		reader--;
		if(reader == 0){
			sem_post(&writer);
		}

		for(int i=0; i<mensajesAEnviar->elements_count; i++){
				mensaje = list_get(mensajesAEnviar, i);
				enviarMensaje(suscripcion, mensaje, socket_cliente, id_proceso);
				if(esCorrelativo(mensaje->id_mensaje_correlativo)){
					id_msg_log = mensaje->id_mensaje_correlativo;
				}else id_msg_log = mensaje->id_mensaje;
				pthread_mutex_lock(&mutex_list_mensaje);
				list_add(mensaje->suscriptoresALosQueSeEnvio, suscriptor);
				pthread_mutex_unlock(&mutex_list_mensaje);
				//esperar ACK:
				if(recv(socket_cliente, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						log_info(logger, "Se recibio el ACK del mensaje id: %d", id_msg_log);
						pthread_mutex_lock(&mutex_list_mensaje);
						list_add(mensaje->suscriptoresQueRecibieron, suscriptor);
						pthread_mutex_unlock(&mutex_list_mensaje);
					} else printf("Luego de enviar el mensaje devolvieron una operacion que no era ACK\n");
				} else {
					log_info(logger, "El proceso id: %d se desconecto", id_proceso);
					liberar_conexion(socket_cliente);
					fin = true;
					break;
				}
		}
	}
	list_destroy(mensajesAEnviar);
}

void manejoMensaje(t_estructura_hilo_mensaje * estructura_mensaje){
	int32_t socket_cliente = estructura_mensaje->socket_cliente;
	int32_t id_proceso = estructura_mensaje->id_proceso;
	int32_t id_mensaje = estructura_mensaje->id_mensaje;
	int32_t operacion = estructura_mensaje->operacion;
	pthread_mutex_unlock(&mutex_estructura_mensajes);
	info_mensaje * mensaje;

	switch(operacion){
	case NEW_POKEMON:
		mensaje = recibirMensajeNew(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count-1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_NEW){
				sem_post(&primerNew);
			}
		}
		sem_post(&primerNew);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	case APPEARED_POKEMON:
		mensaje = recibirMensajeAppeared(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count-1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_APPEARED){
				sem_post(&primerApp);
			}
		}
		sem_post(&primerApp);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	case GET_POKEMON:
		mensaje = recibirMensajeGet(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count-1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_GET){
				sem_post(&primerGet);
			}
		}
		sem_post(&primerGet);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	case LOCALIZED_POKEMON:
		mensaje = recibirMensajeLocalized(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count-1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_LOCALIZED){
				sem_post(&primerLoc);
			}
		}
		sem_post(&primerLoc);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	case CATCH_POKEMON:
		mensaje = recibirMensajeCatch(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count-1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_CATCH){
				sem_post(&primerCatch);
			}
		}
		sem_post(&primerCatch);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	case CAUGHT_POKEMON:
		mensaje = recibirMensajeCaught(socket_cliente);
		if(esCorrelativo(id_mensaje)){
			mensaje->id_mensaje_correlativo = id_mensaje;
		}
		sem_wait(&writer);
		//mostrarEstadoMemoria();
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		guardarMensajeEnCache(mensaje);
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
		//mostrarEstadoMemoria();
		sem_post(&writer);
		pthread_mutex_lock(&mutex_list_mensaje);
		for(int i = 0; i< list_suscriptores->elements_count -1; i++){
			/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
			 * realizar al menos una busqueda
			 */
			t_suscriptor * suscriptor = list_get(list_suscriptores, i);
			if(suscriptor->op_code == SUSCRIPCION_CAUGHT){
				sem_post(&primerCaught);
			}
		}
		sem_post(&primerCaught);
		pthread_mutex_unlock(&mutex_list_mensaje);
		break;
	default:
		break;
	}

	mensaje->process_id=id_proceso;
	enviar_ACK(mensaje->id_mensaje, socket_cliente);
	liberar_conexion(socket_cliente);
}

void guardarMensajeEnCache(info_mensaje* mensaje){
	switch(algMemoria){
	case BS:
		algoritmoBuddySystem(mensaje, algReemplazo);
		break;
	case PARTICIONES:
		algoritmoParticionDinamica(mensaje, frecuenciaCompactacion, algReemplazo, algParticionLibre);
		break;
		}
}

info_mensaje * recibirMensajeNew(int32_t socket_cliente){
	t_New* new = NULL;
	new = deserializar_paquete_new (&socket_cliente);
	log_info(logger, "Llego un mensaje NEW_POKEMON");
	/*printf("Llego un mensaje New Pokemon con los siguientes datos: %d  %s  %d  %d  %d \n", new->pokemon.size_Nombre, new->pokemon.nombre,
			new->cant, new->posicion.X, new->posicion.Y);*/
	info_mensaje * mensajeNew = malloc(sizeof(info_mensaje));
	mensajeNew->op_code = NEW_POKEMON;
	mensajeNew->id_mensaje = get_id();
	mensajeNew->id_mensaje_correlativo = 0;
	mensajeNew->process_id = 1;
	mensajeNew->mensaje = new;
	mensajeNew->sizeMsg = getSizeMensajeNew(*new);
	mensajeNew->suscriptoresALosQueSeEnvio = list_create();
	mensajeNew->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeNew->suscriptoresQueRecibieron = list_create();
	mensajeNew->suscriptoresQueRecibieron->elements_count=0;

	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeNew);
	for(int i = 0; i< list_suscriptores->elements_count-1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_NEW){
		sem_post(&nuevoMensajeNew);
		}
	}
	sem_post(&nuevoMensajeNew);
	pthread_mutex_unlock(&mutex_list_mensaje);


	return mensajeNew;
}


info_mensaje * recibirMensajeAppeared(int32_t socket_cliente){
	t_Appeared* app = NULL;
	app = deserializar_paquete_appeared(&socket_cliente);
	log_info(logger, "Llego un mensaje APPEARED_POKEMON");
	/*printf("Llego un mensaje Appeared Pokemon con los siguientes datos: %d  %s  %d  %d\n", app->pokemon.size_Nombre, app->pokemon.nombre,
			app->posicion.X, app->posicion.Y);*/
	info_mensaje * mensajeAppeared = malloc(sizeof(info_mensaje));
	mensajeAppeared->op_code = APPEARED_POKEMON;
	mensajeAppeared->id_mensaje = get_id();
	mensajeAppeared->id_mensaje_correlativo = 0;
	mensajeAppeared->process_id = 1;
	mensajeAppeared->mensaje = app;
	mensajeAppeared->sizeMsg = getSizeMensajeAppeared(*app);
	mensajeAppeared->suscriptoresALosQueSeEnvio = list_create();
	mensajeAppeared->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeAppeared->suscriptoresQueRecibieron = list_create();
	mensajeAppeared->suscriptoresQueRecibieron->elements_count=0;
	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeAppeared);
	for(int i = 0; i< list_suscriptores->elements_count-1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_APPEARED){
		sem_post(&nuevoMensajeApp);
		}
	}
	sem_post(&nuevoMensajeApp);
	pthread_mutex_unlock(&mutex_list_mensaje);
	return mensajeAppeared;
}

info_mensaje * recibirMensajeGet(int32_t socket_cliente){
	t_Get* get = NULL;
	get = deserializar_paquete_get(&socket_cliente);
	log_info(logger, "Llego un mensaje GET_POKEMON");
	/*printf("Llego un mensaje Get Pokemon con los siguientes datos: %d  %s\n", get->pokemon.size_Nombre, get->pokemon.nombre);*/
	info_mensaje * mensajeGet = malloc(sizeof(info_mensaje));
	mensajeGet->op_code = GET_POKEMON;
	mensajeGet->id_mensaje = get_id();
	mensajeGet->id_mensaje_correlativo = 0;
	mensajeGet->process_id = 1;
	mensajeGet->mensaje = get;
	mensajeGet->sizeMsg = getSizeMensajeGet(*get);
	mensajeGet->suscriptoresALosQueSeEnvio = list_create();
	mensajeGet->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeGet->suscriptoresQueRecibieron = list_create();
	mensajeGet->suscriptoresQueRecibieron->elements_count=0;
	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeGet);
	for(int i = 0; i< list_suscriptores->elements_count-1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_GET){
		sem_post(&nuevoMensajeGet);
		}
	}
	sem_post(&nuevoMensajeGet);
	pthread_mutex_unlock(&mutex_list_mensaje);
	return mensajeGet;
}

info_mensaje * recibirMensajeLocalized(int32_t socket_cliente){
	t_Localized* localized = NULL;
	localized = deserializar_paquete_localized(&socket_cliente);
	log_info(logger, "Llego un mensaje LOCALIZED_POKEMON");

	info_mensaje * mensajeLocalized = malloc(sizeof(info_mensaje));
	mensajeLocalized->op_code = LOCALIZED_POKEMON;
	mensajeLocalized->id_mensaje = get_id();
	mensajeLocalized->id_mensaje_correlativo = 0;
	mensajeLocalized->process_id = 1;
	mensajeLocalized->mensaje = localized;
	mensajeLocalized->sizeMsg = getSizeMensajeLocalized(*localized);
	mensajeLocalized->suscriptoresALosQueSeEnvio = list_create();
	mensajeLocalized->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeLocalized->suscriptoresQueRecibieron = list_create();
	mensajeLocalized->suscriptoresQueRecibieron->elements_count=0;
	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeLocalized);
	for(int i = 0; i< list_suscriptores->elements_count-1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_LOCALIZED){
		sem_post(&nuevoMensajeLoc);
		}
	}
	sem_post(&nuevoMensajeLoc);
	pthread_mutex_unlock(&mutex_list_mensaje);
	return mensajeLocalized;
}

info_mensaje * recibirMensajeCatch(int32_t socket_cliente){
	t_Catch* catch = NULL;
	catch = deserializar_paquete_catch(&socket_cliente);
	log_info(logger, "Llego un mensaje CATCH_POKEMON");
	/*printf("Llego un mensaje Catch Pokemon con los siguientes datos: %d  %s  %d  %d \n", catch->pokemon.size_Nombre, catch->pokemon.nombre,
			catch->posicion.X, catch->posicion.Y);*/
	info_mensaje * mensajeCatch = malloc(sizeof(info_mensaje));
	mensajeCatch->op_code = CATCH_POKEMON;
	mensajeCatch->id_mensaje = get_id();
	mensajeCatch->id_mensaje_correlativo = 0;
	mensajeCatch->process_id = 1;
	mensajeCatch->mensaje = catch;
	mensajeCatch->sizeMsg = getSizeMensajeCatch(*catch);
	mensajeCatch->suscriptoresALosQueSeEnvio = list_create();
	mensajeCatch->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeCatch->suscriptoresQueRecibieron = list_create();
	mensajeCatch->suscriptoresQueRecibieron->elements_count=0;
	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeCatch);
	for(int i = 0; i< list_suscriptores->elements_count-1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_CATCH){
		sem_post(&nuevoMensajeCatch);
		}
	}
	sem_post(&nuevoMensajeCatch);
	pthread_mutex_unlock(&mutex_list_mensaje);
	return mensajeCatch;
}

info_mensaje * recibirMensajeCaught(int32_t socket_cliente){
	t_Caught* caught = NULL;
	caught = deserializar_paquete_caught(&socket_cliente);
	log_info(logger, "Llego un mensaje CAUGHT_POKEMON");
	/*printf("Llego un mensaje Caught Pokemon con los siguientes datos:  %d\n", caught->fueAtrapado);*/
	info_mensaje * mensajeCaught = malloc(sizeof(info_mensaje));
	mensajeCaught->op_code = CAUGHT_POKEMON;
	mensajeCaught->id_mensaje = get_id();
	mensajeCaught->id_mensaje_correlativo = 0;
	mensajeCaught->process_id = 1;
	mensajeCaught->mensaje = caught;
	mensajeCaught->sizeMsg = getSizeMensajeCaught(*caught);
	mensajeCaught->suscriptoresALosQueSeEnvio = list_create();
	mensajeCaught->suscriptoresALosQueSeEnvio->elements_count=0;
	mensajeCaught->suscriptoresQueRecibieron = list_create();
	mensajeCaught->suscriptoresQueRecibieron->elements_count=0;
	pthread_mutex_lock(&mutex_list_mensaje);
	list_add(list_mensajes, mensajeCaught);
	for(int i = 0; i< list_suscriptores->elements_count -1; i++){
		/* "-1" para que por mas que no haya suscriptores pueda garantizar que cuando se suscriba un proceso va a
		 * realizar al menos una busqueda
		 */
		t_suscriptor * suscriptor = list_get(list_suscriptores, i);
		if(suscriptor->op_code == SUSCRIPCION_CAUGHT){
		sem_post(&nuevoMensajeCaught);
		}
	}
	sem_post(&nuevoMensajeCaught);
	pthread_mutex_unlock(&mutex_list_mensaje);
	return mensajeCaught;
}

void enviarMensaje(op_code operacion, info_mensaje * mensaje, int32_t socket_cliente, int32_t id_proceso){
	int32_t id_mensaje;
	t_New * new;
	t_Appeared * app;
	t_Get * get;
	t_Localized * loc;
	t_Catch * catch;
	t_Caught * caught;
	char* x;
	char* y;
	char* cant;
	char* id;
	char* atrapado;

	if(esCorrelativo(mensaje->id_mensaje_correlativo)){
		id_mensaje = mensaje->id_mensaje_correlativo;
	}else id_mensaje = mensaje->id_mensaje;

	switch(operacion){
	case SUSCRIPCION_NEW:
		new = mensaje->mensaje;
		x = string_itoa(new->posicion.X);
		y = string_itoa(new->posicion.Y);
		cant = string_itoa(new->cant);
		id = string_itoa(id_mensaje);
		enviar_new_pokemon(new->pokemon.nombre,x,y,cant,id, socket_cliente);
		log_info(logger, "Se envio un mensaje NEW_POKEMON con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		free(x);
		free(y);
		free(cant);
		free(id);
		break;
	case SUSCRIPCION_APPEARED:
		app = mensaje->mensaje;
		x = string_itoa(app->posicion.X);
		y = string_itoa(app->posicion.Y);
		id = string_itoa(id_mensaje);
		enviar_appeared_pokemon(app->pokemon.nombre, x, y, id, socket_cliente);
		log_info(logger, "Se envio un mensaje APPEARED_POKEMON con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		free(x);
		free(y);
		free(id);
		break;
	case SUSCRIPCION_GET:
		get = mensaje->mensaje;
		id = string_itoa(id_mensaje);
		enviar_get_pokemon(get->pokemon.nombre,id, socket_cliente);
		log_info(logger, "Se envio un mensaje GET_POKEMON con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		free(id);
		break;
	case SUSCRIPCION_LOCALIZED:
		loc = mensaje->mensaje;
		enviar_localized_pokemon(&(loc->pokemon), loc->listaPosiciones, id_mensaje, socket_cliente);
		log_info(logger, "Se envio un mensaje LOCALIZED con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		break;
	case SUSCRIPCION_CATCH:
		catch = mensaje->mensaje;
		x = string_itoa(catch->posicion.X);
		y = string_itoa(catch->posicion.Y);
		id = string_itoa(id_mensaje);
		enviar_catch_pokemon(catch->pokemon.nombre, x, y, id, socket_cliente);
		log_info(logger, "Se envio un mensaje CATCH_POKEMON con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		free(x);
		free(y);
		free(id);
		break;
	case SUSCRIPCION_CAUGHT:
		caught = mensaje->mensaje;
		id = string_itoa(id_mensaje);
		atrapado = string_itoa(caught->fueAtrapado);
		enviar_caught_pokemon(id, atrapado, socket_cliente);
		log_info(logger, "Se envio un mensaje CAUGHT_POKEMON con id: %d al proceso %d", id_mensaje, id_proceso);
		if(algReemplazo==LRU) actualizarID(mensaje->id_mensaje);
		free(id);
		free(atrapado);
		break;
	default:
		break;
	}
}

bool esCorrelativo(int32_t id_mensaje){
	if(id_mensaje != 0){
		return true;
	}
	return false;
}

void iniciarBroker(){
	config = leer_config();
	logger = iniciar_logger();

	log_info(logger, "Iniciando Broker");
	unsigned int miPid= process_getpid();
	log_info(logger, "El PID de Broker es %d", miPid);

	//Lectura de archivo de configuracion
	sizeMemoria = atoi(config_get_string_value(config, "TAMANO_MEMORIA"));
	sizeMinParticion = atoi(config_get_string_value(config, "TAMANO_MINIMO_PARTICION"));

	char * value;
	value= config_get_string_value(config, "ALGORITMO_MEMORIA");
	if(strcmp(value,"BS") == 0){
		algMemoria=BS;
	}else algMemoria=PARTICIONES;

	value = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	if(strcmp(value,"FIFO") == 0){
		algReemplazo=FIFO;
	}else algReemplazo=LRU;

	value = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	if(strcmp(value,"FF") == 0){
		algParticionLibre=FF;
	}else algParticionLibre=BF;

	IP_BROKER = config_get_string_value(config, "IP_BROKER");
	PUERTO_BROKER = config_get_string_value(config, "PUERTO_BROKER");
	frecuenciaCompactacion = atoi(config_get_string_value(config, "FRECUENCIA_COMPACTACION"));
	LOG_FILE = config_get_string_value(config, "LOG_FILE");

	//Inicializar memoria
	inicioMemoria = (int32_t)malloc(sizeMemoria); //f00X12345  f00X12345 + 2048
	t_particion* particionInicial = crearParticion(0, sizeMemoria, false);
	tabla_particiones = list_create();
	list_add(tabla_particiones, particionInicial);

	//Crear lista de suscriptores y mensajes
	list_suscriptores = list_create();
	list_mensajes = list_create();

	pthread_mutex_init(&mutex_id_mensaje, NULL);
	pthread_mutex_init(&mutex_list_mensaje, NULL);
	pthread_mutex_init(&mutex_list_suscriptores, NULL);
	pthread_mutex_init(&mutex_guardar_en_memoria, NULL);
	pthread_mutex_init(&mutex_estructura_mensajes, NULL);
	sem_init(&writer,0,1);
	sem_init(&nuevoMensajeNew,0,0);
	sem_init(&nuevoMensajeApp,0,0);
	sem_init(&nuevoMensajeGet,0,0);
	sem_init(&nuevoMensajeLoc,0,0);
	sem_init(&nuevoMensajeCaught,0,0);
	sem_init(&nuevoMensajeCatch,0,0);
	sem_init(&primerNew,0,0);
	sem_init(&primerApp,0,0);
	sem_init(&primerGet,0,0);
	sem_init(&primerLoc,0,0);
	sem_init(&primerCaught,0,0);
	sem_init(&primerCatch,0,0);

}

t_log* iniciar_logger(void){
	t_log* logger;
	LOG_FILE= config_get_string_value(config, "LOG_FILE");
	logger = log_create(LOG_FILE, "Broker", 1, LOG_LEVEL_INFO);
	if(logger == NULL){
		printf("No pude iniciar el logger\n");
		exit(1);
	}
	log_info(logger, "Inicio log");
	return logger;
}

t_config * leer_config(void){
	t_config * config;
	config = config_create("/home/utnso/workspace/tp-2020-1c-5rona/Broker/Broker.config");
	if(config== NULL){
		printf("No pude leer la config\n");
		exit(2);
	}
	return config;
}

int getMemoriaOcupada(){
	int memoriaOcupada = 0;

	int i = 0;
	for(i = 0; i < tabla_particiones->elements_count; i++){
		t_particion* particion_actual = list_get(tabla_particiones, i);
		memoriaOcupada += particion_actual->size;
	}

	return memoriaOcupada;
}

int getMemoriaDisponible(){
	return sizeMemoria - getMemoriaOcupada();
}

int32_t getSizePokemon(t_pokemon pokemon){
	return sizeof(typeof(pokemon.size_Nombre)) + sizeof(typeof(char)) * pokemon.size_Nombre -1; //-1 para restar el centinela
}

int32_t getSizeMensajeNew(t_New msgNew){
	/*
	 * ‘Pikachu’ 5 10 2
	 * largo del nombre del pokémon
	 * el nombre del pokemon
	 * posición X
 	 * posicion Y
	 * cantidad
	*/
	int32_t sizeMsg = 0;
	sizeMsg +=getSizePokemon(msgNew.pokemon) +
			sizeof(typeof(t_posicion)) +
			sizeof(typeof(msgNew.cant));

	return sizeMsg;
}

int32_t getSizeMensajeLocalized(t_Localized msgLocalized){
	/*
	 * ‘Pikachu’ 3 4 5 1 5 9 3
	 * el largo del nombre del pokémon,
	 * el nombre del pokemon,
	 * la cantidad de posiciones donde se encuentra
	 * y un par de int_32 para cada posición donde se encuentre. (2 * int_32 * cant_posiciones)
	*/

	int32_t sizeMsg = 0;
	sizeMsg += getSizePokemon(msgLocalized.pokemon) + sizeof(typeof(msgLocalized.listaPosiciones->elements_count));
	if(msgLocalized.listaPosiciones->elements_count != 0){
		sizeMsg += sizeof(typeof(t_posicion)) * msgLocalized.listaPosiciones->elements_count;
	}

	return sizeMsg;
}

int32_t getSizeMensajeGet(t_Get msgGet){
	/*
	 * ‘Pikachu’
	 * el largo del nombre del pokémon,
	 * el nombre del pokemon,
	*/

	int32_t sizeMsg = 0;
	sizeMsg += getSizePokemon(msgGet.pokemon);

	return sizeMsg;
}

int32_t getSizeMensajeAppeared(t_Appeared msgAppeared){
	/*
	 * ‘Pikachu’ 1 5
	 * el largo del nombre del pokémon,
	 * el nombre del pokemon,
	 * Pos X
	 * Pos Y
	*/

	int32_t sizeMsg = 0;
	sizeMsg += getSizePokemon(msgAppeared.pokemon)+
				  sizeof(typeof(t_posicion));

	return sizeMsg;
}

int32_t getSizeMensajeCatch(t_Catch msgCatch){
	/*
	 * ‘Pikachu’ 1 5
	 * el largo del nombre del pokémon,
	 * el nombre del pokemon,
	 * Pos X
	 * Pos Y
	*/

	int32_t sizeMsg = 0;
	sizeMsg += getSizePokemon(msgCatch.pokemon)+
				  sizeof(typeof(t_posicion));

	return sizeMsg;
}

int32_t getSizeMensajeCaught(t_Caught msgCaught){
	/*
	 * 0
	 * un uint_32 para saber si se puedo o no atrapar al pokemon
	*/

	return sizeof(typeof(sizeof(msgCaught.fueAtrapado)));
}

void rutina (int n){
	dump = iniciar_dump();
	if(n == SIGUSR1){
		pthread_mutex_lock(&mutex_guardar_en_memoria);
		hacerDump();
		pthread_mutex_unlock(&mutex_guardar_en_memoria);
	}else printf("Signal no reconocida. \n");
}


t_log* iniciar_dump(void){
	t_log* dump;
	dump = log_create("/home/utnso/workspace/tp-2020-1c-5rona/Broker/Broker-dump.log", "Broker", 1, LOG_LEVEL_INFO);
	if(dump == NULL){
		printf("No pude iniciar el dump\n");
		exit(1);
	}
	return dump;
}

void hacerDump(){
	t_particion *particion;
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fecha[128];
    strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);

	log_info(dump, "Dump: %s", fecha);
	int i;
	for(i=0; i<tabla_particiones->elements_count; i++){
		particion = list_get(tabla_particiones, i);
		char estado[] = "L";
		if(particion->ocupada == 1) estado[0] = 'X';
		char cola[9];
		switch(particion->codigo_operacion){
		case NEW_POKEMON:
			strcpy(cola, "NEW");
			break;
		case APPEARED_POKEMON:
			strcpy(cola, "APPEARED");
			break;
		case GET_POKEMON:
			strcpy(cola, "GET");
			break;
		case LOCALIZED_POKEMON:
			strcpy(cola, "LOCALIZED");
			break;
		case CATCH_POKEMON:
			strcpy(cola, "CATCH");
			break;
		case CAUGHT_POKEMON:
			strcpy(cola, "CAUGHT");
			break;
		default:
			strcpy(cola, "VACIA");
			break;
		}
		log_info(dump, "Particion %d: %p - %p.	[%s]\n Size: %db	LRU: %d		COLA: %s		ID_MENSAJE: %d",
				i, inicioMemoria + particion->posicion_inicial, inicioMemoria + particion->posicion_final, estado, particion->size, particion->id,
				cola, particion->id_mensaje);
		//%06p
	}
}

t_list * getMensajesAEnviar(op_code operacion, int32_t id_proceso){
	info_mensaje * mensaje;
	t_particion * mensajeCacheado;
	t_list* mensajesAEnviar = NULL;
	t_list* mensajesCacheados = NULL;
	op_code tipoMensajeABuscar;
	switch(operacion){
	case SUSCRIPCION_NEW:
		tipoMensajeABuscar =NEW_POKEMON;
		break;
	case SUSCRIPCION_APPEARED:
		tipoMensajeABuscar =APPEARED_POKEMON;
		break;
	case SUSCRIPCION_CATCH:
		tipoMensajeABuscar =CATCH_POKEMON;
		break;
	case SUSCRIPCION_CAUGHT:
		tipoMensajeABuscar =CAUGHT_POKEMON;
		break;
	case SUSCRIPCION_GET:
		tipoMensajeABuscar =GET_POKEMON;
		break;
	case SUSCRIPCION_LOCALIZED:
		tipoMensajeABuscar =LOCALIZED_POKEMON;
		break;
	default:
		break;
	}

	mensajesCacheados = getMensajesCacheadosDeOperacion(tipoMensajeABuscar); // mensajesCacheadoes es una lista de t_particion

	mensajesAEnviar=list_create();
	for(int i=0; i<mensajesCacheados->elements_count; i++){
		mensajeCacheado = list_get(mensajesCacheados, i);
		if(!recibioMensaje(id_proceso, mensajeCacheado->id_mensaje)){
			mensaje = obtenerMensaje(mensajeCacheado->id_mensaje);
			list_add(mensajesAEnviar, mensaje);
		}
	}
	list_destroy(mensajesCacheados);
	return mensajesAEnviar;
}

bool recibioMensaje(int32_t id_proceso, int32_t id_mensaje){
	info_mensaje * mensaje;
	t_suscriptor * suscriptor;
	mensaje = obtenerMensaje(id_mensaje); // obtenemos el info_mensaje de list_mensajes

	for(int j=0; j<mensaje->suscriptoresQueRecibieron->elements_count; j++){
		suscriptor = list_get(mensaje->suscriptoresQueRecibieron, j);

		if(suscriptor->id == id_proceso){
			return true;
		}
	}
	return false;
}

bool mensajeCacheadoDeOperacion(t_particion * particion, op_code operacion){
	return particion->codigo_operacion == operacion;
}

t_list* getMensajesCacheadosDeOperacion(op_code operacion){

	bool _mensajeCacheadoDeOperacion(void* element){
		return mensajeCacheadoDeOperacion((t_particion*)element, operacion);
	}
	return list_filter(tabla_particiones, _mensajeCacheadoDeOperacion);
}

bool esElMensaje(info_mensaje* mensaje, int32_t id_mensaje){
	return mensaje->id_mensaje == id_mensaje;
}

info_mensaje * obtenerMensaje(int32_t id_mensaje){
	info_mensaje * mensaje = NULL;

	bool _esElMensaje(void* element){
		return esElMensaje((info_mensaje*)element, id_mensaje);
	}
	pthread_mutex_lock(&mutex_list_mensaje);
	t_list* mensajesConEseID = list_filter(list_mensajes, _esElMensaje);
	pthread_mutex_unlock(&mutex_list_mensaje);

	if(mensajesConEseID->elements_count == 1){
		mensaje = list_get(mensajesConEseID, 0);
		list_destroy(mensajesConEseID);
		return mensaje;
	}else {
		if(mensajesConEseID->elements_count > 1){
			printf("Mas de un mensaje con el mismo id. Cantidad: %d \n", mensajesConEseID->elements_count);
			list_destroy(mensajesConEseID);
			return NULL;
		}else printf("No estaba el mensaje en memoria cache (particiones)");
		list_destroy(mensajesConEseID);
		return NULL;
	}
}

bool esElSuscriptor(t_suscriptor * suscriptor, int32_t id_proceso){
	return suscriptor->id == id_proceso;
}

bool procesoSuscriptoACola(op_code operacion, int32_t id_proceso){
	bool _esElSuscriptor(void* element){
		return esElSuscriptor((t_suscriptor*)element, id_proceso);
	}

	t_list * suscripciones = list_filter(list_suscriptores, _esElSuscriptor);
	t_suscriptor * suscriptor = NULL;
	for(int i = 0; i<suscripciones->elements_count; i++){
		suscriptor = list_get(suscripciones, i);
		if(suscriptor->op_code == operacion){
			list_destroy(suscripciones);
			return true;
		}
	}
	list_destroy(suscripciones);
	return false;
}

t_suscriptor * obtenerSuscriptor(int32_t id_proceso, op_code operacion){
	bool _esElSuscriptor(void* element){
		return esElSuscriptor((t_suscriptor*)element, id_proceso);
	}

	t_list * suscripciones = list_filter(list_suscriptores, _esElSuscriptor);
	for(int i = 0 ; i<suscripciones->elements_count; i++){
		t_suscriptor * suscriptor = list_get(suscripciones, i);
		if(suscriptor->op_code == operacion){
			list_destroy(suscripciones);
			return suscriptor;
		}
	}
	return NULL;
}



