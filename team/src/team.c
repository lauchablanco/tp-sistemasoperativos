/*
 ============================================================================
 Name        : team.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "team.h"
#include "pokemon_utils.h"

//constantes una vez que se inicialicen
int32_t cantidad_entrenadores = 0;
int32_t socket_escucha_team;
int RETARDO_CICLO_CPU = 0;

int ciclos_totales = 0; //ciclos de intercambios + ciclos de cada entrenador por moverse + mensajes a broker
int deadlocks_resueltos_totales = 0; // deadlocks resueltos
int cambios_contexto = 0; //el primer proceso que se carga no cuenta como cambio de contexto
int total_deadlock_resueltos = 0;
int proceso_ejecutando = -1;

int DL_nodo_inicial = -1;
int DL_nodo_final = -1;
int intentos = 0;
t_deadlock* deadlock_actual;

t_list* total_deadlocks;

bool cambio_cola_ready = false;

//colas de planificacion
t_list* cola_ready;
t_list* entrenadores_DL;

//semaforos
sem_t s_cola_ready_con_items;
sem_t s_procesos_en_exec;
sem_t s_posiciones_a_mover;
sem_t s_control_planificador_rr;
sem_t s_detectar_deadlock;
sem_t s_replanificar;
sem_t s_entrenador_exit;
sem_t s_replanificar_sjfcd;
sem_t s_suscripcion_localized;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_ciclos_totales;
pthread_mutex_t mutex_cambios_contexto;
pthread_mutex_t mutex_deadlocks_totales;
pthread_mutex_t mutex_deadlocks_resueltos_totales;



//colas de mensajes
t_list* pokemones_recibidos; // registro para saber si rechazar appeareds y localizeds
t_list* pokemones_ubicados; // estos son los pokemones capturables. Esta lista varía con el tiempo.
t_list* mensajes_get_esperando_respuesta; // seguramente algun id o algo
t_list* mensajes_catch_esperando_respuesta; // seguramente algun id o algo


//void show_estado(){
//
//	//printf("PLANIFICACION: \n	ALGORITMO: %s, QUANTUM: %d\n", algoritmo.algoritmo_string, algoritmo.quantum);
//	    //printf("********************\n");
//
//
//	    void _mostrar_pokemon(void* elemento){
//	    	return mostrar_pokemon((t_pokemon_team*)elemento, objetivo_global);
//	    }
//
//
//	    for(int32_t i = 0; i < entrenadores->elements_count; i++){
//
//	    	t_entrenador* entrenador_actual = list_get(entrenadores, i);
//
//	    	//printf("|ENTRENADOR: %d\n|----------------\n|POSICION: (%d,%d)\n",
//	    			i,
//					entrenador_actual->posicion.X,
//					entrenador_actual->posicion.Y);
//	    	//printf("|ESTADO: %d\n", entrenador_actual->estado);
//	    	if(entrenador_actual->ocupado) //printf("|ESTA OCUPADO\n");
//
//	    	//printf("|POKEMONES:\n");
//	    	list_iterate(entrenador_actual->pokemones, _mostrar_pokemon);
//
//	    	//printf("|OBJETIVOS:\n");
//	    	list_iterate(entrenador_actual->objetivo, _mostrar_pokemon);
//
//	    	//printf("|CICLOS UTILIZADOS:%d\n", entrenador_actual->ciclos);
//			//printf("********************\n");
//
//		}
//
//		//printf("|OBJETIVO GLOBAL:\n");
//		list_iterate(objetivo_global, _mostrar_pokemon);
//
//
//		//printf("|CICLOS TOTALES UTILIZADOS: %d\n", ciclos_totales);
//		//printf("|CAMBIOS DE CONTEXTO: %d\n", cambios_contexto);
//		//printf("********************\n");
//
//		return;
//}


char* get_metricas_string(){
	char* metricas = string_new();
	string_append_with_format(&metricas, "\n\nMETRICAS FINALES ALGORITMO %s\n---------------------\n",algoritmo.algoritmo_string);

	string_append(&metricas, "CANTIDAD DE CICLOS DE CPU POR ENTRENADOR:\n");

	for(int i =0; i < entrenadores->elements_count; i++){

		t_entrenador* entrenador = list_get(entrenadores, i);

		string_append_with_format(&metricas, " * ENTRENADOR %d: %d\n",entrenador->id, entrenador->ciclos);
	}

	string_append_with_format(&metricas, "CANTIDAD DE CICLOS DE CPU TOTALES: %d\n", ciclos_totales);
	string_append_with_format(&metricas, "CANTIDAD DE CAMBIOS DE CONTEXTO: %d\n", cambios_contexto);
	string_append_with_format(&metricas, "CANTIDAD DE DEADLOCKS DETECTADOS: %d\n", total_deadlocks->elements_count);
	string_append_with_format(&metricas, "CANTIDAD DE DEADLOCKS RESUELTOS: %d\n", total_deadlock_resueltos);

	return metricas;
}

void hilo_exit(){
	for(int i =0; i < entrenadores->elements_count;i++){
		sem_wait(&s_entrenador_exit);
	}

	//printf("Todos los entrenadores cumplieron sus objetivos\n");

	for(int i =0; i<entrenadores->elements_count;i++){

		t_entrenador* e = list_get(entrenadores, i);

		ciclos_totales += e->ciclos;
	}

	//show_estado();
	char* metricas = get_metricas_string();
	log_info(logger, metricas);
	free(metricas);

	return;
}


/*AGREGAR QUE TIENEN QUE ESTAR UNIDOS*/
void resolver_deadlock(t_deadlock* deadlock){


	int* e1 = list_get(deadlock->procesos_involucrados, 0);
	t_entrenador* entrenadorDL1 = list_get(entrenadores_DL, *e1);

	t_entrenador* entrenadorDL2 = malloc(sizeof(t_entrenador));

	for(int i = 1; i < entrenadores_DL->elements_count; i++){
		int* e2 = list_get(deadlock->procesos_involucrados, i);
		t_entrenador* entrenadorDL = list_get(entrenadores_DL, *e2);

		if(estan_unidos(entrenadorDL1, entrenadorDL)){
			entrenadorDL2 = entrenadorDL;
		}
	}
	t_entrenador* entrenador1= list_get(entrenadores, entrenadorDL1->id);
	t_entrenador* entrenador2= list_get(entrenadores, entrenadorDL2->id);

	if((entrenador1->estado ==BLOCKED && entrenador1->ocupado) || entrenador1->estado == READY){
		//printf("el entrenador %d ya está resolviendo su deadlock\n", entrenador1->id);
		return;
	}

	if((entrenador2->estado ==BLOCKED && entrenador2->ocupado) || entrenador2->estado == READY){
		//printf("el entrenador %d ya está resolviendo su deadlock\n", entrenador1->id);
		return;
	}

	t_pokemon_team* pokemon_destino = pokemon_que_sirve(entrenadorDL1, entrenadorDL2);

	pokemon_destino->posicion = entrenador2->posicion;

	entrenador1->pokemon_destino=pokemon_destino;

	entrenador1->estado = READY;


	pthread_mutex_lock(&mutex_cola_ready);
	list_add(cola_ready, entrenador1);
	cambio_cola_ready = true;
	pthread_mutex_unlock(&mutex_cola_ready);

	log_info(logger, "El entrenador %d entró a la cola de READY para ir a intercambiar",
								entrenador1->id);

	sem_post(&s_cola_ready_con_items);

	return;
}

void resolver_deadlocks(t_list* deadlocks_encontrados){


	t_deadlock* deadlock = list_get(deadlocks_encontrados, 0);
	resolver_deadlock(deadlock);

	return;
}

bool esta_en_deadlock(void* entrenador){
	t_entrenador* e = (t_entrenador*)entrenador;
	return (e->estado == BLOCKED &&
			!e->ocupado &&
			!cumplio_objetivo(e) &&
			!puede_capturar_pokemones(e));
}




t_list* entrenadores_en_deadlock(){

	t_list* en_DL = list_filter(entrenadores, esta_en_deadlock);


	t_list* entrenadores_en_DL = list_create();
	for(int i = 0; i < en_DL->elements_count; i++){
		t_entrenador* actual = list_get(en_DL, i);
		t_entrenador* nuevo = malloc(sizeof(t_entrenador));
		nuevo->id = actual->id;
		nuevo->pokemones = pokemones_de_mas(actual);//pokemones que no estan en objetivo;
		nuevo->objetivo = objetivos_pendientes(actual);//objetivo que no estan satisfechos;
		list_add(entrenadores_en_DL, nuevo);
	}

	//list_destroy(en_DL);
	return entrenadores_en_DL;

}


bool estan_unidos(t_entrenador* nodo_actual, t_entrenador* nodo_siguiente){
	//estan unidos si el nodo actual necesita algo que tiene el nodo_siguiente
	//nodo_actual.objetivo tiene los objetivos pendientes de satisfacer
	//nodo_siguiente.pokemones tiene los pokemones que tiene de mas

	if(pokemon_que_sirve(nodo_actual, nodo_siguiente) != NULL) return true;

	return false;

}

bool ordenar_DL(void* proceso1, void* proceso2){
	return *((int*)proceso1) < *((int*)proceso2);
}

bool deadlock_ya_detectado(t_list* deadlock_detectados, t_deadlock* deadlock){

	for(int i = 0; i < deadlock_detectados->elements_count; i++){
		t_deadlock* dl = list_get(deadlock_detectados, i);

		if(dl->procesos_involucrados->elements_count != deadlock->procesos_involucrados->elements_count) continue;

		for(int j=0; j < deadlock->procesos_involucrados->elements_count ; j++){
			int* proceso_involucrado = list_get(deadlock->procesos_involucrados, j);
			int* proceso_dl_involucrado = list_get(dl->procesos_involucrados, j);

			if((*proceso_involucrado) == (*proceso_dl_involucrado)){
				if(j == deadlock->procesos_involucrados->elements_count - 1){
					return true;
				}
				continue;
			} else {
				break;
			}
		}
	}

	return false;

}



void detectar_deadlocks(){
	bool puede_haber_deadlock =false;


	for(int i = 0; i< entrenadores->elements_count; i++){
		t_entrenador* e = list_get(entrenadores,i);
		if(!esta_en_deadlock(e) && !(e->estado == EXIT)){
			continue; // no hay deadlock
		} else {
			puede_haber_deadlock =true;
		}
	}

	if(!puede_haber_deadlock) return;

	//printf("puede haber deadlock, analizando...\n");

	entrenadores_DL = entrenadores_en_deadlock();

	if(entrenadores_DL->elements_count <= 1) return;

	log_info(logger, "Inicio algoritmo de detección de Deadlock");

	int filas = entrenadores_DL->elements_count;
	int columnas = filas;

	/* matriz dependiendo la cantidad de entrenadores en DL: si hay 2, será de 2x2
	 *
	 * ej: grafo de deadlock triple
	 *
	 *      A	 B
	 * A|   0  | 1	|
	 * B|	1  | 0	|
	 *
	 * 		PIK 	SQRT 	ONIX
	 * A |   1
	 * B |
	 * C |
	 *
	 *
	 * 	   A B C
	 *  A| 0 1 0
		B| 1 0 1
		C| 0 1 0
	 *
	 * 0: 2pik
	 *1: 1sqr, 1onix
	 *
	*/




	int matriz[filas][columnas];

	for(int i = 0; i < filas; i++){
		t_entrenador* e1 = list_get(entrenadores_DL, i);

		for(int j = 0; j < columnas; j++){
			t_entrenador* e2 = list_get(entrenadores_DL, j);
			if(i==j){
				matriz[i][j] = 0;
			} else {
				matriz[i][j] = estan_unidos(e1, e2);
			}
		}
	}

//	for(int i = 0; i < filas; i++){
//		for(int j = 0; j < columnas; j++){
//			//printf("%d ", matriz[i][j]);
//		}
//		 //printf("\n");
//	}


	t_list* deadlocks_detectados = list_create();


	deadlock_actual = malloc(sizeof(t_deadlock));
	deadlock_actual->procesos_involucrados = list_create();

	bool espera_circular = false;
	void recorrer_fila_DL(int fila){


		for(int j = 0; j < entrenadores->elements_count; j++){

			if(fila==j)continue;
			if(matriz[fila][j]){
			//	//log_debug(logger, "encontré un 1!");
				intentos++;
				int* p = malloc(sizeof(int));
				*p = fila;

				list_add(deadlock_actual->procesos_involucrados, p);

				DL_nodo_final = j;

				//log_debug(logger, "entrenador inicial %d y final %d", DL_nodo_inicial, DL_nodo_final);

				if(DL_nodo_final == DL_nodo_inicial){

					int* p2 = malloc(sizeof(int));
					*p2 = j;

					list_add(deadlock_actual->procesos_involucrados, p2);

					espera_circular = true;
					intentos=0;

					t_deadlock* deadlock_encontrado = malloc(sizeof(t_deadlock));
					deadlock_encontrado->procesos_involucrados = list_create();
					*deadlock_encontrado->procesos_involucrados = *deadlock_actual->procesos_involucrados;

					list_sort(deadlock_encontrado->procesos_involucrados, ordenar_DL);
					deadlock_encontrado->procesos_involucrados = list_eliminar_int_repetidos(deadlock_encontrado->procesos_involucrados);

					if(!deadlock_ya_detectado(deadlocks_detectados, deadlock_encontrado)){
						list_add(deadlocks_detectados, deadlock_encontrado);

						char* dl_string = string_new();

						string_append(&dl_string, "Deadlock encontrado entre los procesos: ");

						for(int i =0; i < deadlock_encontrado->procesos_involucrados->elements_count; i++){

							int* proceso_involucrado = list_get(deadlock_encontrado->procesos_involucrados, i);

							t_entrenador* e = list_get(entrenadores_DL, *proceso_involucrado);
							t_entrenador* eo = list_get(entrenadores, e->id);

							string_append_with_format(&dl_string, " * %d ", eo->id);
						}

						log_info(logger, dl_string);

						free(dl_string);
					}


					deadlock_actual->procesos_involucrados=list_create();



					return;

				} else if (intentos < entrenadores_DL->elements_count){
					recorrer_fila_DL(j);
					return;
				} else {

					log_info(logger, "El algoritmo terminó y no detectó espera circular");
					espera_circular = false;
					return;
				}

			}

		}

	}


	for(int i = 0; i < filas; i++){

		DL_nodo_inicial = i;
		recorrer_fila_DL(i);
	}

	for(int m = 0; m < deadlocks_detectados->elements_count; m++){
		t_deadlock* dl = list_get(deadlocks_detectados, m);


		if(!deadlock_ya_detectado(total_deadlocks, dl)){
			t_deadlock* deadl = malloc(sizeof(t_deadlock));
			deadl->procesos_involucrados = list_create();
			deadl->procesos_involucrados = dl->procesos_involucrados;
			list_add(total_deadlocks, deadl);
		}

		char* log_deadlock = string_new();

		string_append(&log_deadlock, "se detectó un deadlock entre el entrenador: ");

		for(int n=0; n < dl->procesos_involucrados->elements_count ; n++){
			int* proceso_involucrado = list_get(dl->procesos_involucrados, n);
			t_entrenador* entrenador_involucrado = list_get(entrenadores_DL, *proceso_involucrado);
			string_append_with_format(&log_deadlock, "%d ", entrenador_involucrado->id);
		}

		free(log_deadlock);

		//string_append(&log_deadlock, "\n");

		//log_debug(logger, log_deadlock);

	}

	resolver_deadlocks(deadlocks_detectados);
	//list_destroy(deadlocks_detectados);

}



void ejecuta(t_entrenador* entrenador){

	int32_t posicion_final_X = entrenador->pokemon_destino->posicion.X - entrenador->posicion.X;
	int32_t posicion_final_Y = entrenador->pokemon_destino->posicion.Y - entrenador->posicion.Y;

	entrenador->ciclos++;
	entrenador->cantidad_ejecutada++;
	sleep(RETARDO_CICLO_CPU);
	if(posicion_final_X != 0){
		if(posicion_final_X < 0){
			entrenador->posicion = avanzar(entrenador->posicion, -1, 0);
		} else {
			entrenador->posicion = avanzar(entrenador->posicion, 1, 0);
		}

		//printf("**Avanzo 1 paso en X**\n");

		return;

	}

	if(posicion_final_Y != 0){
		if(posicion_final_Y < 0){
			entrenador->posicion = avanzar(entrenador->posicion, 0, -1);
		} else {
			entrenador->posicion = avanzar(entrenador->posicion, 0, 1);
		}
		//printf("**Avanzo 1 paso en Y**\n");

		return;

	}


}


void replanificar_entrenador(t_entrenador* entrenador){
	//printf("VOY A REPLANIFICAR AL ENTRENADOR %d\n", entrenador->id);

	if(entrenador->pokemon_destino == NULL){
		entrenador->pokemon_destino = get_pokemon_necesario_mas_cercano(pokemones_ubicados, entrenador->posicion);
	}

	if(entrenador->pokemon_destino == NULL){
		//printf("No hay pokemones ubicados, me bloqueo\n");

	} else {
		//printf("SU NUEVO POKEMON ES: %s\n", entrenador->pokemon_destino->nombre);
		entrenador->pokemon_destino->planificable = false;
		entrenador->estado = READY;
		pthread_mutex_lock(&mutex_cola_ready);
		list_add(cola_ready, entrenador);
		cambio_cola_ready = true;
		pthread_mutex_unlock(&mutex_cola_ready);
		log_info(logger, "El entrenador %d fue agregado a la cola de READY ya que puede seguir capturando", entrenador->id);
		sem_post(&s_cola_ready_con_items);
	}
}

bool generar_y_enviar_catch(t_entrenador* entrenador){

	int32_t socket = conexion_broker();

	if(socket == 0){
		log_error(logger,"Error al conectar al Broker...");
		//logica de reintentar
		return false;
	}

	int32_t operacion = 0;
	int32_t id_mensaje = 0; // esto creo que habria que cambiarlo
	int32_t tamanio_estructura = 0;

	////printf("Conectado al Broker para enviar CATCH");
	enviar_handshake(PROCESS_ID, socket);
	if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
		if(operacion == ACK){ // Confirmacion de que la identificacion (handshake) fue recibida
			recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
			recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
			////printf("El broker me reconocio\n");

			char* pokemon = entrenador->pokemon_destino->nombre;
			char* posX = string_itoa(entrenador->pokemon_destino->posicion.X);
			char* posY = string_itoa(entrenador->pokemon_destino->posicion.Y);

			enviar_catch_pokemon(pokemon, posX, posY, string_itoa(0), socket);

			if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){// Esperamos confirmacion de recepcion del mensaje
				if(operacion == ACK){
					recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL); //recibo el paquete, aca llega el id_mensaje asignado por Broker


					//log_debug(logger, "espero respuesta %d", id_mensaje);

					t_respuesta* respuesta = malloc(sizeof(t_respuesta));
					respuesta->id_respuesta = id_mensaje;
					respuesta->id_entrenador = entrenador->id;
					list_add(mensajes_catch_esperando_respuesta, respuesta);

					pthread_mutex_lock(&mutex_ciclos_totales);
					ciclos_totales++;
					pthread_mutex_unlock(&mutex_ciclos_totales);

					sleep(RETARDO_CICLO_CPU);
				}
			}
		}
	}

	liberar_conexion(socket);

	return true;
}

void planificar_fifo(){

	//printf("planificando FIFO...\n");


	// en fifo, el proximo entrenador es el que esté primero en la cola de ready
	pthread_mutex_lock(&mutex_cola_ready);
	t_entrenador* entrenador = list_remove(cola_ready, 0);
	pthread_mutex_unlock(&mutex_cola_ready);

	log_info(logger, "El próximo entrenador es %d porque es el primero en la cola de READY", entrenador->id);


	entrenador->estado=EXEC;
	entrenador->ocupado=true;
	sem_post(entrenador->semaforo);

	return;
}

void planificar_rr(){

	pthread_mutex_lock(&mutex_cola_ready);
	t_entrenador* entrenador = list_remove(cola_ready, 0);
	pthread_mutex_unlock(&mutex_cola_ready);

	log_info(logger, "El próximo entrenador es %d porque es el primero en la cola", entrenador->id);

	sem_post(entrenador->semaforo); // por cada paso que tiene que dar

	return;
}

int get_index_entrenador_estimacion_mas_corta(){

	// FORMULA DE ESTIMACION: ESTn+1 = alpha * TEn + (1-alpha) * ESTn

	//TE: t ejec rafaga actual
	//EST: estimado de la rafaga actual

	double estimacion_mas_corta = -1;
	int indice = 0;


	for(int i = 0; i < cola_ready->elements_count; i++){

		pthread_mutex_lock(&mutex_cola_ready);
		t_entrenador* entrenador_actual = list_get(cola_ready, i);
		pthread_mutex_unlock(&mutex_cola_ready);



		double EST_siguiente = 0;

		if(entrenador_actual->estimacion_anterior == 0){
			EST_siguiente = entrenador_actual->estimacion;
		} else {

			int TE = get_distancia_entre_puntos(entrenador_actual->posicion, entrenador_actual->pokemon_destino->posicion);
			double EST = entrenador_actual->estimacion_anterior;

			EST_siguiente =  algoritmo.alpha * TE + (1-algoritmo.alpha) * EST;
			entrenador_actual->estimacion = EST_siguiente;
		}

		//log_debug(logger, "estimacion entrenador %d: %d", entrenador_actual->id, entrenador_actual->estimacion);

		if(estimacion_mas_corta == -1){
			estimacion_mas_corta = EST_siguiente;
		} else if(EST_siguiente < estimacion_mas_corta) {
			estimacion_mas_corta = EST_siguiente;
			indice = i;
		}
	}

	return indice;
}

void planificar_sjfsd(){

	int i = get_index_entrenador_estimacion_mas_corta();
	pthread_mutex_lock(&mutex_cola_ready);
	t_entrenador* entrenador = list_remove(cola_ready, i);
	pthread_mutex_unlock(&mutex_cola_ready);

	log_info(logger, "El próximo entrenador es %d porque su estimación es la mas corta (%d)",
						entrenador->id, entrenador->estimacion - entrenador->cantidad_ejecutada);

	sem_post(entrenador->semaforo);
	return;
}

void planificar_sjfcd(){
	planificar_sjfsd();
	return;
}

void planificar()
{

	switch(algoritmo.algoritmo_code){
	case FIFO:
		planificar_fifo();
		break;
	case RR:
		planificar_rr();
		break;
	case SJFSD:
		planificar_sjfsd();
		break;
	case SJFCD:
		planificar_sjfcd();
		break;
	default:
		return;
	}

}


void hilo_replanificar(){

	while(1){
		//printf("soy el hilo que replanifica! esperando...\n");
		sem_wait(&s_replanificar);
		//printf("llegó algo pa replanificar\n");
		t_entrenador* replanificar = malloc(sizeof(t_entrenador));
		for(int i = 0; i < entrenadores->elements_count; i++){
			t_entrenador* entrenador = list_get(entrenadores, i);
			if(puede_capturar_pokemones(entrenador) && entrenador->estado == BLOCKED && !entrenador->ocupado && entrenador->pokemon_destino == NULL){

				replanificar = entrenador;
			}
		}

		replanificar_entrenador(replanificar);
	}

}

void hilo_replanificar_sjfcd(){

	while(1){
		//printf("soy el hilo que replanifica sjfcd! esperando que llegue algo a ready...\n");
		sem_wait(&s_replanificar_sjfcd);

		//printf("llegó algo pa replanificar y el algoritmo es SJF-CD\n");
		cambio_cola_ready = false;

		//t_entrenador* replanificar = malloc(sizeof(t_entrenador));

		//t_entrenador* entrenador_en_exec =

		//printf("el proceso ejecutando es :%d\n", proceso_ejecutando);
		t_entrenador* entrenador_ejecutando = list_get(entrenadores, proceso_ejecutando);

		int i = get_index_entrenador_estimacion_mas_corta();

		pthread_mutex_lock(&mutex_cola_ready);
		t_entrenador* entrenador = list_get(cola_ready, i);
		pthread_mutex_unlock(&mutex_cola_ready);



		if(entrenador_ejecutando->estimacion - entrenador_ejecutando->cantidad_ejecutada <= entrenador->estimacion){
			//printf("sigue ejecutando el entrenador actual\n");
			entrenador_ejecutando->estado=EXEC;
			sem_post(entrenador_ejecutando->semaforo);
		} else {
			log_info(logger, "El próximo entrenador es %d porque su estimación es la mas corta (%d)",
					entrenador->id, entrenador->estimacion);
			entrenador_ejecutando->estado = READY;
			entrenador->estado = EXEC;
			sem_post(entrenador_ejecutando->semaforo);
			sem_post(entrenador->semaforo);
		}
	}
}

void hilo_detectar_deadlock(){
	while(1){
		//printf("soy el hilo que detecta deadlocks! esperando...\n");
		for(int i = 0; i < entrenadores->elements_count; i++){
			sem_wait(&s_detectar_deadlock);
		}

		//printf("puede que haya deadlock..\n");
		//show_estado();
		detectar_deadlocks();
	}
}

void capturar(t_entrenador* entrenador){
	log_info(logger, "Intentando atrapar al pokémon %s ubicado en la posición (%d, %d)",
			entrenador->pokemon_destino->nombre,
			entrenador->pokemon_destino->posicion.X,
			entrenador->pokemon_destino->posicion.Y);
	if(!generar_y_enviar_catch(entrenador)){//esto quiere decir que no se pudo conectar al broker

		list_add(entrenador->pokemones, entrenador->pokemon_destino);
		entrenador->pokemon_destino = NULL;
		entrenador->cantidad_ejecutada = 0;
		entrenador->estimacion_anterior = entrenador->estimacion;
		entrenador->estimacion = 0;

		entrenador->ocupado = false;

		log_info(logger, "El entrenador %d capturó el pokémon", entrenador->id);

		if(!cumplio_objetivo(entrenador)){
			//printf("el entrenador %d aún no cumplio sus objetivos, pasandolo a BLOCKED\n", entrenador->id);

			if(puede_capturar_pokemones(entrenador)){
				//printf("el entrenador puede capturar mas pokemones, llamando al replanificador...\n");
				sem_post(&s_replanificar);//replanificar_entrenador(entrenador);
			} else {
				//printf("el entrenador no puede capturar mas pokemones, queda bloqueado hasta intercambiar\n");
				sem_post(&s_detectar_deadlock);
			}
		}
	} else {
		//printf("ESPERANDO A VER SI LO CAPTURÉ!\n");
		sem_wait(entrenador->semaforo);

		if(!cumplio_objetivo(entrenador)){
			//printf("el entrenador %d aún no cumplio sus objetivos, pasandolo a BLOCKED\n", entrenador->id);

			if(puede_capturar_pokemones(entrenador)){
				//printf("el entrenador puede capturar mas pokemones, llamando al replanificador...\n");
				sem_post(&s_replanificar);//replanificar_entrenador(entrenador);
			} else {
				//printf("el entrenador no puede capturar mas pokemones, queda bloqueado hasta intercambiar\n");
				sem_post(&s_detectar_deadlock);
			}
		}
	}
}

t_entrenador* get_entrenador_intercambio(t_entrenador* entrenador_para_intercambio){

	for(int i = 0; i < entrenadores->elements_count; i++){
		//pthread_mutex_lock(&mutex_list_entrenadores);
		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		//pthread_mutex_unlock(&mutex_list_entrenadores);
		if(entrenador_actual->id == entrenador_para_intercambio->id) continue;

		if(entrenador_actual->posicion.X == entrenador_para_intercambio->posicion.X &&
				entrenador_actual->posicion.Y == entrenador_para_intercambio->posicion.Y){

			t_pokemon_team* pokemon = pokemon_que_sirve_intercambio(entrenador_para_intercambio, entrenador_actual);

			if(pokemon != NULL){
				return entrenador_actual;
			}
		}
	}

	return NULL;
}


void intercambio(t_entrenador* entrenador){
	//printf("Soy el entrenador %d y voy a intercambiar con el que esté en esta posicion\n", entrenador->id);
	//puedo tener mas de uno dependiendo cuantos participan en el deadlock.
	//show_estado();
	t_entrenador* entrenador_en_posicion = get_entrenador_intercambio(entrenador);

	log_info(logger, "El entrenador %d va a intercambiar con el entrenador %d", entrenador->id, entrenador_en_posicion->id);

	t_pokemon_team* pokemon_queme_sirve = entrenador->pokemon_destino; // es el entrenador->pokemon_destino
	t_pokemon_team* pokemon_quele_sirve = pokemon_que_sirve_intercambio(entrenador_en_posicion, entrenador);

	if(pokemon_quele_sirve != NULL){
		//printf("el pokemon que le sirve al otro es %s\n", pokemon_quele_sirve->nombre);
	} else {

		//printf("Tiene algo que me sirve pero yo no tengo nada que le sirva\n");
		//printf("Le doy un pokemon random que me sobre\n");

		t_list* pokemons_de_mas = pokemones_de_mas(entrenador);

		if(pokemons_de_mas->elements_count == 0){
			//printf("no le puedo dar nada porque no me sobra nada\n");
		} else {
			pokemon_quele_sirve = list_get(pokemons_de_mas, 0);
			//printf("le voy a dar el pokemon %s\n", pokemon_quele_sirve->nombre);

		}

	}

	entrenador_en_posicion->pokemon_destino = pokemon_quele_sirve;

	if(entrenador_en_posicion != NULL){
		//printf("llegué al entrenador que tiene el pokemon que me interesa\n");
		entrenador_en_posicion->ocupado=true;

		if(pokemon_queme_sirve != NULL && pokemon_quele_sirve != NULL){

			//printf("ambos entrenadores tienen algo que le interesa al otro\n");

			//printf("intercambiando primer pokemon:\n");

			for(int i =0; i < 5; i++){
				//printf("%d ciclo de cpu\n", i + 1);
				pthread_mutex_lock(&mutex_ciclos_totales);
				ciclos_totales++;
				pthread_mutex_unlock(&mutex_ciclos_totales);
				sleep(RETARDO_CICLO_CPU);
			}

			list_add(entrenador->pokemones, pokemon_queme_sirve);
			remove_pokemon_by_nombre(pokemon_queme_sirve->nombre, entrenador_en_posicion->pokemones);//remuevo de la lista de pokemones el que le di al entrenador
			entrenador->pokemon_destino=NULL;
			list_add(entrenador_en_posicion->pokemones, entrenador_en_posicion->pokemon_destino);
			remove_pokemon_by_nombre(pokemon_quele_sirve->nombre, entrenador->pokemones);
			entrenador_en_posicion->pokemon_destino=NULL;


			//printf("INTERCAMBIO FINALIZADO!\n");


			if(!esta_en_deadlock((void*)entrenador) || !esta_en_deadlock((void*)entrenador_en_posicion) ){
				total_deadlock_resueltos++;
			}


			entrenador->estado = BLOCKED;
			entrenador->ocupado = false;
			entrenador_en_posicion->estado = BLOCKED;
			entrenador_en_posicion->ocupado = false;
			sem_post(entrenador->semaforo);
			sem_post(entrenador_en_posicion->semaforo);
		}


	} else {
		//printf("**hubo un error porque no hay otro entrenador en esta posicion**\n");
	}

	detectar_deadlocks();
}


void capturar_pokemon(t_entrenador* entrenador){

	//log_debug(logger, "EL ENTRENADOR %d LLEGÓ A DESTINO!! X: %d, Y: %d, \n",entrenador->id, entrenador->posicion.X, entrenador->posicion.Y);
	entrenador->estado=BLOCKED;
	entrenador->ocupado=true;

	sem_post(&s_procesos_en_exec); // salgo de exec

	if(puede_capturar_pokemones(entrenador)){
		capturar(entrenador);
	} else {
		intercambio(entrenador);//intercambio, hay cambio de contexto?
	}
}

void ejecutar_fifo(t_entrenador* entrenador){

	while(entrenador->posicion.X != entrenador->pokemon_destino->posicion.X ||
			entrenador->posicion.Y != entrenador->pokemon_destino->posicion.Y){

		ejecuta(entrenador);
		log_info(logger, "El entrenador %d se movió a la posición: (%d, %d)", entrenador->id, entrenador->posicion.X, entrenador->posicion.Y);

	}

	capturar_pokemon(entrenador);

}

void ejecutar_rr(t_entrenador* entrenador){

	int cantidad_a_moverse = get_distancia_entre_puntos(entrenador->posicion, entrenador->pokemon_destino->posicion);
	int movimientos = cantidad_a_moverse < algoritmo.quantum ? cantidad_a_moverse : algoritmo.quantum;

	for(int i = 0; i < movimientos; i++){

		ejecuta(entrenador);
		log_info(logger, "El entrenador %d se movió a la posición: (%d, %d)", entrenador->id, entrenador->posicion.X, entrenador->posicion.Y);
	}


	cantidad_a_moverse = get_distancia_entre_puntos(entrenador->posicion, entrenador->pokemon_destino->posicion);
	if(cantidad_a_moverse > 0 ){
	//printf("se terminó el quantum y todavía falta que se mueva, lo mando a la cola de READY\n");

		entrenador->estado = READY;

		pthread_mutex_lock(&mutex_cola_ready);
		list_add(cola_ready, entrenador);
		cambio_cola_ready = true;
		pthread_mutex_unlock(&mutex_cola_ready);
		log_info(logger, "El entrenador %d entró a la cola de READY porque se terminó su quantum", entrenador->id);

		sem_post(&s_cola_ready_con_items);
		sem_post(&s_procesos_en_exec);
	} else {
		capturar_pokemon(entrenador);
	}
}

void ejecutar_sjfsd(t_entrenador* entrenador){
	return ejecutar_fifo(entrenador);
}

void ejecutar_sjfcd(t_entrenador* entrenador){

	while(entrenador->posicion.X != entrenador->pokemon_destino->posicion.X ||
			entrenador->posicion.Y != entrenador->pokemon_destino->posicion.Y){
		if(cambio_cola_ready && cola_ready->elements_count > 0){

			sem_post(&s_replanificar_sjfcd);
			sem_wait(entrenador->semaforo);

			if(entrenador->estado == READY){
				return;
			} else if (entrenador->estado == EXEC){
				ejecuta(entrenador);
				log_info(logger, "El entrenador %d se movió a la posición: (%d, %d)", entrenador->id, entrenador->posicion.X, entrenador->posicion.Y);
			}

		} else {
			ejecuta(entrenador);
			log_info(logger, "El entrenador %d se movió a la posición: (%d, %d)", entrenador->id, entrenador->posicion.X, entrenador->posicion.Y);
		}
	}


	capturar_pokemon(entrenador);


	return;
}


void ejecutar_algoritmo(t_entrenador* entrenador)
{

	switch(algoritmo.algoritmo_code){
	case FIFO:
		ejecutar_fifo(entrenador);
		break;
	case RR:
		ejecutar_rr(entrenador);
		break;
	case SJFSD:
		ejecutar_sjfsd(entrenador);
		break;
	case SJFCD:
		ejecutar_sjfcd(entrenador);
		break;
	default:
		return;
	}

}



void hilo_enviar_get(int i){
	int32_t socket = conexion_broker();

	//printf("hilo creado para enviar get...\n");


	t_pokemon_team* pokemon = list_get(objetivo_global, i);

	if(socket == 0){
		log_error(logger,"Error al conectar al Broker para enviar GET...");
		list_add(pokemones_recibidos, pokemon->nombre); // simulo que recibí un localized sin posiciones
		return;
	}

	//log_debug(logger,"Conectado al Broker para enviar GET");

	int32_t operacion = 0;
	int32_t id_mensaje = 0;
	int32_t tamanio_estructura = 0;


	enviar_handshake(PROCESS_ID, socket);

	if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
		if(operacion == ACK){
			recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
			recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

			enviar_get_pokemon(pokemon->nombre, "0", socket);



			if(recv(socket, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
				if(operacion == ACK){
					recv(socket, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

					t_respuesta* respuesta = malloc(sizeof(t_respuesta));
					respuesta->id_respuesta = id_mensaje;
					respuesta->id_entrenador = 0;
					//log_debug(logger, "espero el ID: %d", id_mensaje);

					list_add(mensajes_get_esperando_respuesta, respuesta);

					pthread_mutex_lock(&mutex_ciclos_totales);
					ciclos_totales++;
					pthread_mutex_unlock(&mutex_ciclos_totales);

					sleep(RETARDO_CICLO_CPU);

				}
			}
		}
	}

	liberar_conexion(socket);
}

void generar_y_enviar_get(){
	for(int i = 0; i < objetivo_global->elements_count; i++){

		/* HILO PLANIFICADOR */
//		pthread_t p_enviar_get;
//		pthread_create(&p_enviar_get, NULL, (void*)hilo_enviar_get, (void*)i);
//		pthread_detach(p_enviar_get);

		hilo_enviar_get(i);

	}

	return;
}

//void show_cola_ready(){
//	for(int i = 0; i < cola_ready->elements_count; i++){
//		pthread_mutex_lock(&mutex_cola_ready);
//		t_entrenador* entrenador_actual = list_get(cola_ready, i);
//		pthread_mutex_unlock(&mutex_cola_ready);
//
//		//printf("posicion en la lista: %d\n", i);
//		//printf("posicion actual: X:%d, Y:%d\n", entrenador_actual->posicion.X, entrenador_actual->posicion.Y);
//		//printf("posicion destino: X:%d, Y:%d\n", entrenador_actual->pokemon_destino->posicion.X, entrenador_actual->pokemon_destino->posicion.Y);
//	}
//}

void hilo_planificador(){

	while(1){
		sem_wait(&s_cola_ready_con_items);//inicializado en 0
		//printf("llegó algo a la cola de READY\n");
		sem_wait(&s_procesos_en_exec); //inicializado en 1, o sea que solo puede haber uno a la vez;
		planificar();
	}
}




void ubicar_pokemones_localized(t_Localized* pokemones_a_ubicar){
	for(int i = 0; i < pokemones_a_ubicar->listaPosiciones->elements_count; i++){

		t_posicion* posicion = list_get(pokemones_a_ubicar->listaPosiciones, i);
		t_pokemon_team* pokemon_ubicado = get_pokemon_team(pokemones_a_ubicar->pokemon.nombre, *posicion);

		list_add(pokemones_ubicados, pokemon_ubicado);
	}
}

void recibidor_mensajes_localized(void* args){
	t_args_mensajes* arg = (t_args_mensajes*)args;

	if(arg->respuesta->id_entrenador == -1){
		//log_debug(logger, "No me interesa el msj lo descarto!\n");
		return;
	}

	t_Localized* mensaje = (t_Localized*)arg->mensaje;
	t_respuesta* respuesta = arg->respuesta;

	//log_debug(logger, "Se recibió un LOCALIZED %s (id: %d)", mensaje->pokemon.nombre, respuesta->id_respuesta);

	if(localized_valido(mensaje, respuesta->id_respuesta, mensajes_get_esperando_respuesta, pokemones_recibidos, objetivo_global)){
		//log_debug(logger, "A WILD %s WAS LOCALIZED!!!!\n", mensaje->pokemon.nombre);
		list_add(pokemones_recibidos, mensaje->pokemon.nombre);

		//tengo que planificar tantas posiciones como pokemon necesite.
		//Ej, si me llegan 3 posiciones y necesito 2, solo planifico 2 pero las otras las guardo en memoria
		//Si necesito 2 posiciones y llega 1, planifico 1.

		int cantidad_pokemones_global = get_cantidad_by_nombre_pokemon(mensaje->pokemon.nombre, objetivo_global);


		// me quedo con la menor de las cantidades
		int cantidad_a_planificar = 0;
		if(mensaje->listaPosiciones->elements_count <= cantidad_pokemones_global ){
			cantidad_a_planificar = mensaje->listaPosiciones->elements_count;
		} else {
			cantidad_a_planificar = cantidad_pokemones_global;
		}

		//log_debug(logger, "voy a ubicar %d pokemones %s", cantidad_a_planificar, mensaje->pokemon.nombre);

		for(int i = 0; i < cantidad_a_planificar; i++){
			t_posicion* posicion = list_get(mensaje->listaPosiciones, i);

			t_entrenador* entrenador_mas_cercano = get_entrenador_planificable_mas_cercano(entrenadores, *posicion);

			if(entrenador_mas_cercano != NULL){

				//Si hay entrenadores planificables, planifico esa posicion que siempre va a ser la 0
				list_remove(mensaje->listaPosiciones, 0);

				t_pokemon_team* pokemon_destino = get_pokemon_team(mensaje->pokemon.nombre, *posicion);

				entrenador_mas_cercano->estado = READY;
				entrenador_mas_cercano->pokemon_destino = pokemon_destino;


				pthread_mutex_lock(&mutex_cola_ready);
				list_add(cola_ready, entrenador_mas_cercano);
				cambio_cola_ready = true;
				pthread_mutex_unlock(&mutex_cola_ready);
				log_info(logger, "El entrenador %d entró a la cola de READY ya que es el entrenador mas cercano",
										entrenador_mas_cercano->id);
				sem_post(&s_cola_ready_con_items);
				//sem_post(entrenador_mas_cercano->semaforo);
			} else {
				ubicar_pokemones_localized(mensaje);
			}
		}
	} else {
		//log_debug(logger, "no me sirve");
	}

}

void recibidor_mensajes_appeared(t_Appeared* args){
	t_Appeared* mensaje = NULL;
	memcpy(&mensaje, args, sizeof(t_Appeared*));

	if(appeared_valido(mensaje, entrenadores, objetivo_global)){
		//printf("A WILD %s APPEARED!!!!\n", mensaje->pokemon.nombre);

		list_add(pokemones_recibidos, mensaje->pokemon.nombre);

		t_entrenador* entrenador_mas_cercano = get_entrenador_planificable_mas_cercano(entrenadores, mensaje->posicion);

		if(entrenador_mas_cercano != NULL){
			entrenador_mas_cercano->estado = READY;

			t_pokemon_team* pokemon_destino = get_pokemon_team(mensaje->pokemon.nombre, mensaje->posicion);
			pokemon_destino->planificable = false;
			entrenador_mas_cercano->pokemon_destino = pokemon_destino;
			entrenador_mas_cercano->ocupado = true;


			pthread_mutex_lock(&mutex_cola_ready);
			list_add(cola_ready, entrenador_mas_cercano);
			cambio_cola_ready = true;
			pthread_mutex_unlock(&mutex_cola_ready);

			log_info(logger, "El entrenador %d entró a la cola de READY ya que es el entrenador mas cercano",
									entrenador_mas_cercano->id);

			sem_post(&s_cola_ready_con_items);

		} else {
			t_pokemon_team* pokemon_ubicado = get_pokemon_team(mensaje->pokemon.nombre, mensaje->posicion);
			pokemon_ubicado->planificable = true;
			list_add(pokemones_ubicados, pokemon_ubicado);

		}
	}

	free(mensaje);

	pthread_exit(NULL);

}

void recibidor_mensajes_caught(void* args){
	t_args_mensajes* arg = (t_args_mensajes*)args;
	t_Caught* mensaje = (t_Caught*)arg->mensaje;
	t_respuesta* respuesta = arg->respuesta;

	if(arg->respuesta->id_entrenador == -1){
		return;
	}

	t_entrenador* entrenador = list_get(entrenadores, respuesta->id_entrenador);

	if(mensaje->fueAtrapado){
		list_add(entrenador->pokemones, entrenador->pokemon_destino);
		log_info(logger, "El entrenador %d capturó el pokémon", entrenador->id);

	} else {
		log_info(logger, "El entrenador %d no capturó el pokémon", entrenador->id);
	}

	entrenador->pokemon_destino = NULL;
	entrenador->ocupado = false;
	sem_post(entrenador->semaforo);

	return;
}

void hilo_recibidor_mensajes_gameboy(){
	while(1){
		int32_t socket_cliente = (int32_t)recibir_cliente(socket_escucha_team);

		if(socket_cliente != -1){

			int32_t codigo_operacion = 0;
			int32_t tamanio_estructura = 0;
			int32_t id_mensaje = 0;

			if(recv(socket_cliente, &codigo_operacion, sizeof(int32_t), MSG_WAITALL) == -1 )
					codigo_operacion = -1;
			recv(socket_cliente, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
			recv(socket_cliente, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

			switch(codigo_operacion){
				case APPEARED_POKEMON:
					;
					t_Appeared* mensaje_appeared = deserializar_paquete_appeared(&socket_cliente);

					t_args_mensajes* args = malloc(sizeof(t_args_mensajes));
					args->mensaje = mensaje_appeared;
					args->respuesta = NULL;

					log_info(logger, "Recibí un mensaje APPEARED con el id %d, pokémon %s, posición: (%d, %d)",
								id_mensaje, mensaje_appeared->pokemon.nombre, mensaje_appeared->posicion.X, mensaje_appeared->posicion.Y);


					pthread_t p_generador_mensajes_appeared;
					pthread_create(&p_generador_mensajes_appeared, NULL, (void*)recibidor_mensajes_appeared, (void*)args);
					pthread_detach(p_generador_mensajes_appeared);

					break;

				default:
					return;
			}
		}
	}
}

char* get_config_path(char* entrenador){
	char* cfg_path = string_new();
	string_append(&cfg_path, "/home/utnso/workspace/tp-2020-1c-5rona/team/config/");
	string_append(&cfg_path, entrenador);
	string_append(&cfg_path, ".config");

	return cfg_path;
}

void hilo_suscriptor_appeared(op_code *code){
	int32_t operacion=0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje=0;
	int32_t socket_broker = crear_conexion(IP_BROKER,PUERTO_BROKER);
	bool fin = false;

	while(1){
		if(socket_broker != 0){
			enviar_handshake(PROCESS_ID, socket_broker);
			if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);


					enviar_suscripcion(*code, socket_broker);
					pthread_mutex_lock(&mutex_ciclos_totales);
					ciclos_totales++;
					pthread_mutex_unlock(&mutex_ciclos_totales);
					sleep(RETARDO_CICLO_CPU);
					if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){
						if(operacion == ACK){
							recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

							while(fin == false){
								if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){

									recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
									recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

									if(operacion == APPEARED_POKEMON){
										t_Appeared* mensaje_appeared = NULL;
										mensaje_appeared = deserializar_paquete_appeared(&socket_broker);
										enviar_ACK(0, socket_broker);
										log_info(logger, "Recibí un mensaje APPEARED con el id %d, pokémon %s, posición: (%d, %d)",
													id_mensaje, mensaje_appeared->pokemon.nombre, mensaje_appeared->posicion.X, mensaje_appeared->posicion.Y);


										t_args_mensajes* args = malloc(sizeof(t_args_mensajes));
										args->mensaje = mensaje_appeared;
										args->respuesta = NULL;

										pthread_t p_generador_mensajes_appeared;
										pthread_create(&p_generador_mensajes_appeared, NULL, (void*) recibidor_mensajes_appeared, (void*)args);
										pthread_detach(p_generador_mensajes_appeared);
									}


								} else {
									log_info(logger, "Se cayo la conexion");
									liberar_conexion(socket_broker);
									socket_broker = 0;
									fin = true;
								}
							}

						} else {
							log_error(logger, "Fallo la suscripcion, respondieron algo que no era un ACK");
							liberar_conexion(socket_broker);
							socket_broker = 0;
						}
					} else {
						log_error(logger, "Se cayo la conexion con Broker");
						liberar_conexion(socket_broker);
						socket_broker = 0;
					}
				} else {
					log_error(logger, "Fallo el ACK del handshake con Broker");
					liberar_conexion(socket_broker);
					socket_broker = 0;
				}

			}

		} else {
			//log_info(logger, "Reintentando conexion cola...");
			socket_broker = reconectar(socket_broker);
			fin = false;
		}
	}
}

void hilo_suscriptor_caught(op_code* code){
	int32_t operacion=0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje=0;
	int32_t socket_broker = crear_conexion(IP_BROKER,PUERTO_BROKER);
	bool fin = false;

	while(1){

		if(socket_broker != 0){
			enviar_handshake(PROCESS_ID, socket_broker);
			if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);


					enviar_suscripcion(*code, socket_broker);
					pthread_mutex_lock(&mutex_ciclos_totales);
					ciclos_totales++;
					pthread_mutex_unlock(&mutex_ciclos_totales);
					sleep(RETARDO_CICLO_CPU);
					if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){
						if(operacion == ACK){
							recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

							while(fin == false){

								if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){

									recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
									recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

									//printf("id mensaje caught recibido %d", id_mensaje);

									if(operacion == CAUGHT_POKEMON){
										t_Caught* mensaje_caught = deserializar_paquete_caught(&socket_broker);
										enviar_ACK(0, socket_broker);
										char * str_respuesta = string_new();
										string_append(&str_respuesta, mensaje_caught->fueAtrapado ? "OK" : "FAIL");
										log_info(logger, "Recibí un mensaje CAUGHT con el id %d y la respuesta es %s", id_mensaje, str_respuesta);

										free(str_respuesta);

										t_respuesta* respuesta_catch = get_respuesta(id_mensaje, mensajes_catch_esperando_respuesta);

										t_args_mensajes* args = malloc(sizeof(t_args_mensajes));
										args->mensaje = mensaje_caught;
										args->respuesta = respuesta_catch;

										pthread_t p_generador_mensajes_caught;
										pthread_create(&p_generador_mensajes_caught, NULL, (void*)recibidor_mensajes_caught, (void*)args);
										pthread_detach(p_generador_mensajes_caught);

									}

								} else {
									log_info(logger, "Se cayo la conexion");
									liberar_conexion(socket_broker);//logica reconectar
									socket_broker = 0;
									fin = true;
								}
							}

						} else {
							log_error(logger, "Fallo la suscripcion, respondieron algo que no era un ACK");
							liberar_conexion(socket_broker);
							socket_broker = 0;
						}
					} else {
						log_error(logger, "Se cayo la conexion con Broker");
						liberar_conexion(socket_broker);
						socket_broker = 0;
					}
				} else {
					log_error(logger, "Fallo el ACK del handshake con Broker");
					liberar_conexion(socket_broker);
					socket_broker = 0;
				}

			}

		} else {
			//log_info(logger, "Reintentando conexion cola...");
			socket_broker = reconectar(socket_broker);
			fin = false;
		}
	}
}

void hilo_suscriptor_localized(op_code* code){
	int32_t operacion=0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje=0;
	int32_t socket_broker = crear_conexion(IP_BROKER,PUERTO_BROKER);
	bool fin = false;

	while(1){

		if(socket_broker != 0){

			enviar_handshake(PROCESS_ID, socket_broker);
			if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){
				if(operacion == ACK){
					sem_post(&s_suscripcion_localized);
					recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
					recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);


					pthread_mutex_lock(&mutex_ciclos_totales);
					ciclos_totales++;
					pthread_mutex_unlock(&mutex_ciclos_totales);
					sleep(RETARDO_CICLO_CPU);
					if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){
						if(operacion == ACK){
							recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
							recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);

							while(fin == false){
								if(recv(socket_broker, &operacion, sizeof(int32_t), MSG_WAITALL) >0){

									recv(socket_broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
									recv(socket_broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);


									t_Localized* mensaje_localized = deserializar_paquete_localized(&socket_broker);


									enviar_ACK(0, socket_broker);

									char* mensaje_log = string_new();
									string_append_with_format(&mensaje_log, "Recibí un mensaje LOCALIZED con el id %d y posiciones: ", id_mensaje);

									if(mensaje_localized->listaPosiciones->elements_count == 0){
										string_append(&mensaje_log, "no hay registros");
									} else {
										for(int i =0; i < mensaje_localized->listaPosiciones->elements_count; i++){

											t_posicion* posicion = list_get(mensaje_localized->listaPosiciones, i);

											string_append_with_format(&mensaje_log, "(%d, %d),", posicion->X, posicion->Y);
										}

									}

									log_info(logger, mensaje_log);

									free(mensaje_log);

									t_respuesta* respuesta_get = get_respuesta(id_mensaje, mensajes_get_esperando_respuesta);


									t_args_mensajes* args = malloc(sizeof(t_args_mensajes));
									args->mensaje = mensaje_localized;
									args->respuesta = respuesta_get;

									pthread_t p_generador_mensajes_localized;
									pthread_create(&p_generador_mensajes_localized, NULL, (void*)recibidor_mensajes_localized, (void*)args);
									pthread_detach(p_generador_mensajes_localized);

								} else {
									log_info(logger, "Se cayo la conexion\n");
									liberar_conexion(socket_broker);//logica reconectar
									socket_broker = 0;
									fin = true;
								}
							}

						} else {
							log_error(logger, "Fallo la suscripcion, respondieron algo que no era un ACK");
							liberar_conexion(socket_broker);
							socket_broker = 0;
						}
					} else {
						log_error(logger, "Se cayo la conexion con Broker");
						liberar_conexion(socket_broker);
						socket_broker = 0;
					}
				} else {
					log_error(logger, "Fallo el ACK del handshake con Broker");
					liberar_conexion(socket_broker);
					socket_broker = 0;
				}

			}

		} else {
			sem_post(&s_suscripcion_localized);
			socket_broker = reconectar(socket_broker);
			fin = false;
		}
	}
}

int inicializar_team(char* entrenador){

	cola_ready = list_create();
	objetivo_global = list_create();
	pokemones_recibidos = list_create();
	pokemones_ubicados = list_create();
	mensajes_get_esperando_respuesta = list_create();
	mensajes_catch_esperando_respuesta = list_create();
	entrenadores_DL = list_create();
	entrenadores = list_create();
	total_deadlocks = list_create();
	sem_init(&s_cola_ready_con_items, 0, 0);
	sem_init(&s_posiciones_a_mover, 0, 0);
	sem_init(&s_procesos_en_exec, 0, 1);
	sem_init(&s_control_planificador_rr, 0, 0);
	sem_init(&s_detectar_deadlock, 0, 0);
	sem_init(&s_replanificar, 0, 0);
	sem_init(&s_entrenador_exit, 0, 0);
	sem_init(&s_replanificar_sjfcd, 0,0);
	sem_init(&s_suscripcion_localized, 0,0);

	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_ciclos_totales, NULL);
	pthread_mutex_init(&mutex_deadlocks_totales, NULL);
	pthread_mutex_init(&mutex_deadlocks_resueltos_totales, NULL);
	pthread_mutex_init(&mutex_cambios_contexto, NULL);


	srand(time(NULL));

	char* config_path = get_config_path(entrenador);
	config = config_create(config_path);

	char* log_path = config_get_string_value(config, "LOG_FILE");
	logger = log_create(log_path, "Team", 1, LOG_LEVEL_DEBUG);

	IP_BROKER = config_get_string_value(config, "IP_BROKER");
	PUERTO_BROKER = config_get_string_value(config, "PUERTO_BROKER");
	PROCESS_ID = atoi(config_get_string_value(config, "PROCESS_ID"));
	RETARDO_CICLO_CPU = atoi(config_get_string_value(config, "RETARDO_CICLO_CPU"));
	algoritmo = get_algoritmo(config);
	TIEMPO_RECONEXION = atoi(config_get_string_value(config,"TIEMPO_RECONEXION"));

	free(log_path);
	free(config_path);
	return 1;
}



void entrenador(void* index){
	t_entrenador* entrenador = list_get(entrenadores, (int)index);
	//NEW
	//printf("este hilo maneja al entrenador %d\n", entrenador->id);
	while(!cumplio_objetivo(entrenador)){

		sem_wait(entrenador->semaforo); //READY, todavia no tengo ningun pokemon asignado
		//log_debug(logger, "soy el entrenador %d y estoy ejecutando\n", entrenador->id);
		if(proceso_ejecutando != entrenador->id){

			pthread_mutex_lock(&mutex_cambios_contexto);
			cambios_contexto++;//cambio de contexto ya que empieza a ejecutar el proceso
			pthread_mutex_unlock(&mutex_cambios_contexto);


			proceso_ejecutando = entrenador->id;
		}

		if(entrenador->pokemon_destino != NULL){

		log_info(logger, "El entrenador %d se mueve a atrapar a %s en posición %d-%d.\n",
				entrenador->id, entrenador->pokemon_destino->nombre,
				entrenador->pokemon_destino->posicion.X,
				entrenador->pokemon_destino->posicion.Y);
			ejecutar_algoritmo(entrenador);
		}

	}

	log_info(logger, "El entrenador %d cumplió todos sus objetivos.\n", entrenador->id);
	entrenador->estado=EXIT;

	sem_post(&s_detectar_deadlock);
	sem_post(&s_entrenador_exit);

}


void liberar_memoria(){

	liberar_elementos_lista_pokemon(objetivo_global);

	liberar_elementos_lista_pokemon(pokemones_recibidos);

	liberar_elementos_lista_pokemon(pokemones_ubicados);
	//liberar_elementos_lista_deadlock(total_deadlocks);

	free(total_deadlocks);; // eliminar los t_deadlocks

	liberar_elementos_lista_entrenador(entrenadores_DL);

	liberar_elementos_lista_entrenador(entrenadores);

	liberar_elementos_lista_entrenador(cola_ready);

	liberar_elementos_lista_respuesta(mensajes_get_esperando_respuesta);

	liberar_elementos_lista_respuesta(mensajes_catch_esperando_respuesta);

	sem_destroy(&s_cola_ready_con_items);
	sem_destroy(&s_procesos_en_exec);
	sem_destroy(&s_posiciones_a_mover);
	sem_destroy(&s_control_planificador_rr);
	sem_destroy(&s_detectar_deadlock);
	sem_destroy(&s_replanificar);
	sem_destroy(&s_entrenador_exit);
	sem_destroy(&s_replanificar_sjfcd);
	sem_destroy(&s_suscripcion_localized);
	pthread_mutex_destroy(&mutex_cola_ready);
	pthread_mutex_destroy(&mutex_ciclos_totales);
	pthread_mutex_destroy(&mutex_deadlocks_totales);
	pthread_mutex_destroy(&mutex_deadlocks_resueltos_totales);
	pthread_mutex_destroy(&mutex_cambios_contexto);
	//config_destroy(config);
	log_destroy(logger);

}

void liberar_strings(char** cadenas) {
	int i = 0;
	while(cadenas[i] != NULL) {
	        free(cadenas[i]);
	        i++;
	}
	free(cadenas);
}


int32_t main(int32_t argc, char** argv){
	if(!argv[1]){
		//printf("Fata definir el team a cargar\n");
		return EXIT_FAILURE;
	}

	printf("Inicia team\n");
	inicializar_team(argv[1]);
	char** array = config_get_array_value(config, "POSICIONES_ENTRENADORES");
    cantidad_entrenadores = array_length(array);
    liberar_strings(array);


    entrenadores = get_entrenadores(config, cantidad_entrenadores);

    objetivo_global = get_objetivo_global(entrenadores);


    pthread_t p_suscribirse_localized;
	op_code op_localized = SUSCRIPCION_LOCALIZED;
	pthread_create(&p_suscribirse_localized, NULL, (void*)&hilo_suscriptor_localized, &op_localized);
	pthread_detach(p_suscribirse_localized);

	sem_wait(&s_suscripcion_localized);


    pthread_t p_suscribirse_appeared;
    op_code op_appeared = SUSCRIPCION_APPEARED;
   	pthread_create(&p_suscribirse_appeared, NULL, (void*)&hilo_suscriptor_appeared, &op_appeared);
   	pthread_detach(p_suscribirse_appeared);

	pthread_t p_suscribirse_caught;
	op_code op_caught = SUSCRIPCION_CAUGHT;
	pthread_create(&p_suscribirse_caught, NULL, (void*)&hilo_suscriptor_caught, &op_caught);
	pthread_detach(p_suscribirse_caught);



	generar_y_enviar_get();


    /* HILO PLANIFICADOR */
    pthread_t p_planificador;
	pthread_create(&p_planificador, NULL, (void*)hilo_planificador, NULL);
	pthread_detach(p_planificador);


    /* HILO DETECTOR DE DEADLOCKS */
    pthread_t p_detectar_deadlock;
	pthread_create(&p_detectar_deadlock, NULL, (void*)hilo_detectar_deadlock, NULL);
	pthread_detach(p_detectar_deadlock);

	 /* HILO DETECTOR DE DEADLOCKS */
	pthread_t p_replanificar;
	pthread_create(&p_replanificar, NULL, (void*)hilo_replanificar, NULL);
	pthread_detach(p_replanificar);

	 /* HILO DETECTOR DE DEADLOCKS */
	pthread_t p_replanificar_sjfcd;
	pthread_create(&p_replanificar_sjfcd, NULL, (void*)hilo_replanificar_sjfcd, NULL);
	pthread_detach(p_replanificar_sjfcd);

	/* UN HILO POR PROCESO */
    for(int x = 0; x < cantidad_entrenadores; x++){
    	pthread_t p_entrenador;
		pthread_create(&p_entrenador, NULL, (void*)entrenador, (void*)x);
		pthread_detach(p_entrenador);
    }

    pthread_t p_escuchador;
    /* HILO ESCUCHADOR DE GAMEBOY */
    char* ip = config_get_string_value(config, "IP");
	char* puerto = config_get_string_value(config, "PUERTO");

	socket_escucha_team = crear_socket_escucha(ip, puerto);

	//printf("Creado socket de escucha \n");

	if(socket_escucha_team == -1){
		log_error(logger, "Fallo al crear socket de escucha = -1\n");
	} else {
	    pthread_create(&p_escuchador, NULL, (void*)hilo_recibidor_mensajes_gameboy, (void*)entrenadores);
	}

	pthread_detach(p_escuchador);



	 /* HILO DE EXITS */
	pthread_t p_exit;
	pthread_create(&p_exit, NULL, (void*)hilo_exit, NULL);
	pthread_join(p_exit, NULL);


	liberar_conexion(socket_escucha_team);

	//liberar_memoria();

    return EXIT_SUCCESS;
}




