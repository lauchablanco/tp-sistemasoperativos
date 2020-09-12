#include "Game-Boy.h"

t_log* logger;
t_config* config;

int32_t main(int32_t argc, char *argv[])
{
	logger = log_create("/home/utnso/workspace/tp-2020-1c-5rona/Game-Boy/Game-Boy.log", "Game-Boy", 1, LOG_LEVEL_INFO);
	config = config_create("/home/utnso/workspace/tp-2020-1c-5rona/Game-Boy/Game-Boy.config");

	validar_Argc(logger,argc);
	int32_t socket;
	int32_t operacion=0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje=0;


	// BROKER:

	if(string_contains(argv[1], "BROKER")){

		socket = conexionBroker();

		if(socket == 0){
			log_error(logger,"Error al conectar al Broker");
			finalizar(logger, config, socket);
			return 0;
		}
		log_info(logger,"Conectado al Broker");

		enviar_handshake(1, socket);

		if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
			if(operacion == ACK){ // Confirmacion de que la identificacion (handshake) fue recibida
				recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
				recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL); //recibo el paquete, aunque a Game Boy no le interesa ningun dato

				if(string_contains(argv[2], "NEW_POKEMON")){
					log_info(logger,"Envio NEW POKEMON");
					enviar_new_pokemon(argv[3], argv[4], argv[5], argv[6], "0", socket);
					if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						if(operacion == ACK){
							recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						}
					}
				}
				if(string_contains(argv[2], "APPEARED_POKEMON")){
					log_info(logger,"Envio APPEARED POKEMON");
					enviar_appeared_pokemon(argv[3], argv[4], argv[5], "0", socket);
					if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						if(operacion == ACK){
							recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						}
					}
				}
				if(string_contains(argv[2], "CATCH_POKEMON")){
					log_info(logger,"Envio CATCH POKEMON");
					enviar_catch_pokemon(argv[3], argv[4], argv[5], "0", socket);
					if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						if(operacion == ACK){
							recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						}
					}
				}
				if(string_contains(argv[2], "CAUGHT_POKEMON")){
					log_info(logger,"Envio CAUGHT POKEMON");
					enviar_caught_pokemon(argv[3], argv[4], socket);
					if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						if(operacion == ACK){
							recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						}
					}
				}
				if(string_contains(argv[2], "GET_POKEMON")){
					log_info(logger,"Envio GET POKEMON");
					enviar_get_pokemon(argv[3], "0", socket);
					if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
						if(operacion == ACK){
							recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
						}
					}
				}
			}
		}
		else printf("Fallo al recibir codigo de operacion = -1\n");
		liberar_conexion(socket);
	}

	// GAME-CARD:

	else if(string_contains(argv[1], "GAMECARD")) {

		socket = conexionGameCard();

		if(socket == 0){
			log_error(logger,"Error al conectar con Game-Card");
			finalizar(logger, config, socket);
			return 0;
		}
		log_info(logger,"Conectado al Game Card");

		if(string_contains(argv[2], "NEW_POKEMON")){
			log_info(logger,"Envio new_pokemon");
			enviar_new_pokemon(argv[3], argv[4], argv[5], argv[6], argv[7], socket);
			if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				}
			}
		}

		if(string_contains(argv[2], "CATCH_POKEMON")){
			log_info(logger,"Envio Catch Pokemon");
			enviar_catch_pokemon(argv[3], argv[4], argv[5], argv[6], socket);
			if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				}
			}
		}

		if(string_contains(argv[2], "GET_POKEMON")){
			log_info(logger,"Envio Get Pokemon");
			enviar_get_pokemon(argv[3], argv[4], socket);
			if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				}
			}
		}
	}

	// TEAM:

	else if(string_contains(argv[1], "TEAM")){

		socket = conexionTeam();

		if(socket == 0)
		{
			log_error(logger,"Error al conectar al Team");
			finalizar(logger, config, socket);
			return 0;
		}
		log_info(logger,"Conectado al Team");
		if(string_contains(argv[2], "APPEARED_POKEMON")){
			enviar_appeared_pokemon(argv[3], argv[4], argv[5], "0", socket);
		}
	}

	// MODO SUSCRIPTOR:
	else if(string_contains(argv[1], "SUSCRIPTOR")){

		socket = conexionBroker();

		if(socket == 0){
			log_error(logger,"Error al conectar al Broker");
			finalizar(logger, config, socket);
			return 0;
		}

		sem_init(&cronometro, 0, 1);

		enviar_handshake(1, socket);
		if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
			if(operacion == ACK){
				recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
				recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);


				if(string_contains(argv[2], "NEW_POKEMON")){
					log_info(logger,"Suscribo a NEW POKEMON");
					operacion = SUSCRIPCION_NEW;
				}if(string_contains(argv[2], "APPEARED_POKEMON")){
					log_info(logger,"Suscribo a APPEARED POKEMON");
					operacion = SUSCRIPCION_APPEARED;
				}if (string_contains(argv[2], "GET_POKEMON")){
					log_info(logger,"Suscribo a GET POKEMON");
					operacion = SUSCRIPCION_GET;
				}if (string_contains(argv[2], "LOCALIZED_POKEMON")){
					log_info(logger,"Suscribo a LOCALIZED POKEMON");
					operacion = SUSCRIPCION_LOCALIZED;
				}if (string_contains(argv[2], "CATCH_POKEMON")){
					log_info(logger,"Suscribo a CATCH POKEMON");
					operacion = SUSCRIPCION_CATCH;
				}if (string_contains(argv[2], "CAUGHT_POKEMON")){
					log_info(logger,"Suscribo a CAUGHT POKEMON");
					operacion = SUSCRIPCION_CAUGHT;
				}

				enviar_suscripcion(operacion, socket);

				if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
					if(operacion == ACK){
						recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

						pthread_t hilo_temporizador;
						t_estructura_cronometro estructura_cronometro;
						estructura_cronometro.tiempo = (int32_t) atoi(argv[3]);
						estructura_cronometro.socket = socket;
						pthread_create(&hilo_temporizador, NULL, (void*)cronometrar, (void*) &estructura_cronometro);

						while(1){
							if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
								recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
								recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
								recibir_mensaje(socket, operacion);
							}
							sem_wait(&cronometro);
							if (!sigue_corriendo) break;
							sem_post(&cronometro);
						}

						sem_destroy(&cronometro);
					}
				}
			}
		}
	}

	finalizar(logger, config, socket);
	return 0;
}

void validar_Argc(t_log* logger, int32_t arg)
{
	if(arg < 3)
	{
		log_error(logger,"Error de argumentos");
		log_destroy(logger);
		exit(1);
	}
}

void finalizar(t_log* logger, t_config* config, int32_t socket)
{
	liberar_conexion(socket);
	config_destroy(config);
	log_destroy(logger);
}

int32_t existe (char* ruta) {
	struct stat estado_archivo;
	return (stat (ruta, &estado_archivo) == 0);
}

void cronometrar(t_estructura_cronometro * estructura_cronometro){
	int32_t tiempo = estructura_cronometro->tiempo;
	int32_t socket = estructura_cronometro->socket;

	sleep(tiempo);
	sem_wait(&cronometro);
	sigue_corriendo = 0;
	sem_post(&cronometro);
	shutdown(socket, SHUT_RDWR);
}

void recibir_mensaje(int32_t socket, op_code operacion){
	t_New* new = NULL;
	t_Appeared* app = NULL;
	t_Get* get = NULL;
	t_Catch* catch = NULL;
	t_Caught* caught = NULL;

	switch(operacion){
	case NEW_POKEMON:
		new = deserializar_paquete_new(&socket);
		log_info(logger, "Recibi un NEW_POKEMON. Pokemon: %s, posicion: (%d, %d), cantidad: %d",new->pokemon.nombre,
				new->posicion.X, new->posicion.Y, new->cant);
		break;
	case APPEARED_POKEMON:
		app = deserializar_paquete_appeared(&socket);
		log_info(logger, "Recibi un APPEARED_POKEMON. Pokemon: %s, posicion: (%d, %d)",app->pokemon.nombre,
				app->posicion.X, app->posicion.Y);
		break;
	case GET_POKEMON:
		get = deserializar_paquete_get(&socket);
		log_info(logger, "Recibi un GET_POKEMON. Pokemon: %s",get->pokemon.nombre);
		break;
	case CATCH_POKEMON:
		catch = deserializar_paquete_catch(&socket);
		log_info(logger, "Recibi un CATCH_POKEMON. Pokemon: %s, posicion: (%d, %d)",catch->pokemon.nombre,
				catch->posicion.X, catch->posicion.Y);
		break;
	case CAUGHT_POKEMON:
		caught = deserializar_paquete_caught(&socket);
		char fueAtrapado[] = "N";
		if(caught->fueAtrapado == 1) fueAtrapado[0] = 'S';
		log_info(logger, "Recibi un CAUGHT_POKEMON. FueAtrapado: %s", fueAtrapado);
		break;
	default:
		break;
	}

	enviar_ACK(0, socket);
}



