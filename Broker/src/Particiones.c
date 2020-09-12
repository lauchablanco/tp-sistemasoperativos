/*
 * Particiones.c
 *
 *  Created on: 22 jun. 2020
 *      Author: utnso
 */

#include "Particiones.h"

#include<commons/string.h>

int32_t id_particion = 0;
pthread_mutex_t mutex_id_particion;

int32_t get_id_particion(){
	pthread_mutex_lock(&mutex_id_particion);
	++id_particion;
	pthread_mutex_unlock(&mutex_id_particion);
	return id_particion;
}

t_particion* crearParticion(int inicio, int size, bool ocupada){
	t_particion* newParticion = malloc(sizeof(t_particion));
	newParticion->ocupada = false;
	newParticion->posicion_inicial = inicio;
	newParticion->posicion_final = inicio+size-1; //para contemplar el 0. Ej: si es de 128bytes arranca en 0 y va a 127
	newParticion->size = size;
	newParticion->id = get_id_particion();
	newParticion->codigo_operacion = -1;
	newParticion->id_mensaje = 0;

	return newParticion;
}

bool particionCandidata(t_particion* particion, int32_t sizeMensaje){
	return !particion->ocupada && sizeMensaje <= particion->size; // && ((particion->size-sizeMensaje) >= sizeMinParticion || (particion->size-sizeMensaje) == 0);
}

bool hayParticionesCandidatas(int32_t sizeMsg){
	bool _particionCandidata(void* element){
		return particionCandidata((t_particion*)element, sizeMsg);
	}
	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidata);
	bool resultado = particionesCandidatas->elements_count != 0;
	list_destroy(particionesCandidatas);
	return resultado;
}


// ALGORITMOS DE SELECCION DE PARTICION LIBRE
t_particion* getParticionFirstFit(int32_t sizeMensaje){

	bool _particionCandidata(void* element){
		return particionCandidata((t_particion*)element, sizeMensaje);
	}

	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidata);
	t_particion* candidato = list_get(particionesCandidatas, 0);
	list_destroy(particionesCandidatas);
	return candidato;
}

t_particion* getParticionBestFit(int32_t sizeMensaje){
	int i;
	t_particion* mejorParticion; // = malloc(sizeof(t_particion));

	bool _particionCandidata(void* element){
		return particionCandidata((t_particion*)element, sizeMensaje);
	}

	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidata);

	mejorParticion = list_get(particionesCandidatas, 0); //agarro la primera

	if(mejorParticion->size - sizeMensaje != 0){ // si la primera que tomamos no es BestFit hacemos toda la logica
		//recorro a partir de la segunda
		for(i = 1; i < particionesCandidatas->elements_count; i++){
			t_particion* particionActual = list_get(particionesCandidatas, i);

			if(particionActual->size - sizeMensaje == 0){
				return particionActual;
			} else if(particionActual->size < mejorParticion->size){
				mejorParticion = particionActual;
			}
		}
	}

	return mejorParticion;

}

void generarParticionDinamica(t_particion* particionOriginal, int32_t sizeMsg){
	int posicion;
	int inicioIzquierda = particionOriginal->posicion_inicial;
	int inicioDerecha = particionOriginal->posicion_inicial + sizeMsg;

	t_particion* primeraParticion = crearParticion(inicioIzquierda, sizeMsg, false);
	sleep(0.25);
	t_particion* segundaParticion = crearParticion(inicioDerecha, particionOriginal->size-primeraParticion->size, false);

	posicion = obtenerPosicion(particionOriginal);
	list_remove_and_destroy_element(tabla_particiones, posicion, free);
	list_add_in_index(tabla_particiones,posicion, segundaParticion);
	list_add_in_index(tabla_particiones,posicion, primeraParticion);

}

bool particionCandidataVictima(t_particion* particion){
	return particion->ocupada;
}

void algoritmoLiberacion(int32_t algoritmoReemplazo){
	t_particion * particion;
	int posicion;

	switch(algoritmoReemplazo){
	case FIFO:
		particion = algoritmoFIFO();
		posicion = obtenerPosicion(particion);
		particion->ocupada= false;
		particion->codigo_operacion =-1;
		particion->id=0;
		particion->id_mensaje =0;
		list_remove(tabla_particiones, posicion);
		list_add_in_index(tabla_particiones,posicion, particion);
		log_info(logger, "Se elimino un mensaje de la memoria. Posicion de inicio: %d", inicioMemoria + particion->posicion_inicial);
		consolidarParticion(particion, posicion);
		break;
	case LRU:
		particion = algoritmoLRU();
		posicion = obtenerPosicion(particion);
		particion->ocupada= false;
		particion->codigo_operacion =-1;
		particion->id=get_id_particion();
		particion->id_mensaje =0;
		list_remove(tabla_particiones, posicion);
		list_add_in_index(tabla_particiones,posicion, particion);
		log_info(logger, "Se elimino un mensaje de la memoria. Posicion de inicio: %d", inicioMemoria + particion->posicion_inicial);
		consolidarParticion(particion, posicion);
		break;
	default:
		printf("Ese algoritmo de liberacion no esta implementado\n");
		return;
	}
}

void algoritmoLiberacionBS(int32_t algoritmoReemplazo){
	t_particion * particion;
	int posicion;

	switch(algoritmoReemplazo){
	case FIFO:
		particion = algoritmoFIFO();
		posicion = obtenerPosicion(particion);
		particion->ocupada= false;
		particion->codigo_operacion =-1;
		particion->id= 0;
		particion->id_mensaje =0;
		list_remove(tabla_particiones, posicion);
		list_add_in_index(tabla_particiones,posicion, particion);
		log_info(logger, "Se elimino un mensaje de la memoria. Posicion de inicio: %d", inicioMemoria + particion->posicion_inicial);
		consolidarParticionBS(particion, posicion);
		break;
	case LRU:
		particion = algoritmoLRU();
		posicion = obtenerPosicion(particion);
		particion->ocupada= false;
		particion->codigo_operacion =-1;
		particion->id=get_id_particion();
		particion->id_mensaje =0;
		list_remove(tabla_particiones, posicion);
		list_add_in_index(tabla_particiones,posicion, particion);
		log_info(logger, "Se elimino un mensaje de la memoria. Posicion de inicio: %d", inicioMemoria + particion->posicion_inicial);
		consolidarParticionBS(particion, posicion);
		break;
	default:
		printf("Ese algoritmo de liberacion no esta implementado\n");
		return;
	}
}

void consolidarParticion(t_particion * particion, int posicion){
	t_particion* particionAMirar;

	if(posicion !=0){//si tiene una particion antes
		particionAMirar = list_get(tabla_particiones, posicion-1);
		if(!particionAMirar->ocupada){//si la particion anterior esta libre
			particion->posicion_inicial = particionAMirar->posicion_inicial;
			particion->size += particionAMirar->size;
			list_remove(tabla_particiones, posicion-1);
			posicion--;
		}
	}
	if(posicion != (tabla_particiones->elements_count)-1){//si tiene una particion despues
		particionAMirar = list_get(tabla_particiones, posicion+1);
		if(!particionAMirar->ocupada){//si la particion siguiente esta libre
			particion->posicion_final = particionAMirar->posicion_final;
			particion->size += particionAMirar->size;
			list_remove(tabla_particiones, posicion+1);
		}
	}
}

void consolidarParticionBS(t_particion * particion, int posicion){
	t_particion* particionAIzquierda;
	t_particion* particionADerecha;

	if(posicion !=0){//si tiene una particion antes
		particionAIzquierda = list_get(tabla_particiones, posicion-1);
		if(!particionAIzquierda->ocupada){//si la particion anterior esta libre
			if(particion->size==particionAIzquierda->size){ //si son del mismo tamanio
				if(particion->posicion_inicial == (particionAIzquierda->posicion_inicial^particion->size)
						&& particionAIzquierda->posicion_inicial == (particion->posicion_inicial^particion->size)){
					int32_t posicion1 = inicioMemoria + particionAIzquierda->posicion_inicial;
					int32_t posicion2 = inicioMemoria + particion->posicion_inicial;

					particion->posicion_inicial = particionAIzquierda->posicion_inicial;
					particion->size += particionAIzquierda->size;

					free(particionAIzquierda);
					list_remove(tabla_particiones, posicion);
					list_remove(tabla_particiones, posicion-1);
					list_add_in_index(tabla_particiones,posicion-1, particion);
					posicion --;

					log_info(logger, "Se consolidaron dos particiones. Posicion de inicio particion 1: %d. Posicion de inicio particion 2: %d", posicion1, posicion2);
				}
			}
		}
	}
	if(posicion != (tabla_particiones->elements_count)-1){//si tiene una particion despues
		particionADerecha = list_get(tabla_particiones, posicion+1);
		if(!particionADerecha->ocupada){//si la particion siguiente esta libre
			if(particion->size==particionADerecha->size){ //si son del mismo tamanio
				if(particionADerecha->posicion_inicial == (particion->posicion_inicial^particion->size)
						&& particion->posicion_inicial == (particionADerecha->posicion_inicial-particion->size)){
					int32_t posicion1 = inicioMemoria + particion->posicion_inicial;
					int32_t posicion2 = inicioMemoria + particionADerecha->posicion_inicial;

					particion->posicion_final = particionADerecha->posicion_final;
					particion->size += particionADerecha->size;

					free(particionADerecha);
					list_remove(tabla_particiones, posicion+1);
					list_remove(tabla_particiones, posicion);
					list_add_in_index(tabla_particiones,posicion, particion);

					log_info(logger, "Se consolidaron dos particiones. Posicion de inicio particion 1: %d. Posicion de inicio particion 2: %d", posicion1, posicion2);
				}
			}
		}
	}
}

void algoritmoCompactacion(){
	int i;
	t_list * particionesOcupadas = list_create();
	t_list * particionesLibres = list_create();
	t_particion * particionActual;
	int32_t posicion = 0; //inicioMemoria;

	for(i=0; i<tabla_particiones->elements_count; i++){
		particionActual = list_get(tabla_particiones, i);
		if(particionActual->ocupada){
			list_add(particionesOcupadas, particionActual);
		} else list_add(particionesLibres, particionActual);
	}
	list_clean(tabla_particiones);

	// GUARDAMOS LAS PARTICIONES OCUPADAS
	for(i=0; i<particionesOcupadas->elements_count; i++){
		particionActual = list_get(particionesOcupadas, i);
		particionActual->posicion_inicial = posicion;
		particionActual->posicion_final = posicion+particionActual->size-1;
		posicion += particionActual->size;
		list_add(tabla_particiones, particionActual);
	}

	// GUARDAMOS LAS PARTICIONES LIBRES
	for(i=0; i<particionesLibres->elements_count; i++){
		particionActual = list_get(particionesLibres, i);
		particionActual->posicion_inicial = posicion;
		particionActual->posicion_final = posicion+particionActual->size-1;
		posicion += particionActual->size;
		list_add(tabla_particiones, particionActual);
	}

	// CONSOLIDAMOS LAS PARTICIONES LIBRES
	int posicionParticion = particionesOcupadas->elements_count; // inicio de las particiones libres, para saber desde donde consolidar
	for(i=0; i < particionesLibres->elements_count-1; i++){
		particionActual = list_get(tabla_particiones, posicionParticion);
		consolidarParticion(particionActual, posicionParticion);
	}

	list_clean(particionesOcupadas);
	list_destroy(particionesOcupadas);
	list_clean(particionesLibres);
	list_destroy(particionesLibres);
	log_info(logger, "Se ejecuto una compactacion.");
}

t_particion * algoritmoFIFO(){ //debemos tomar la de id mas chico
	int i;
	t_particion* particionAEliminar;

	bool _particionCandidataVictima(void* element){
		return particionCandidataVictima((t_particion*)element);
	}

	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidataVictima);

	particionAEliminar = list_get(particionesCandidatas, 0); //agarro la primera
	for(i = 1; i < particionesCandidatas->elements_count; i++){ //comparo con la que sigue
		t_particion* particionActual = list_get(particionesCandidatas, i);
		if(particionAEliminar->id > particionActual->id){
			particionAEliminar = particionActual;
		}
	}
	list_destroy(particionesCandidatas);
	return particionAEliminar;
}

t_particion * algoritmoLRU(){ //debemos tomar la de id mas grande
	int i;
	t_particion* particionAEliminar;

	bool _particionCandidataVictima(void* element){
		return particionCandidataVictima((t_particion*)element);
	}

	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidataVictima);

	particionAEliminar = list_get(particionesCandidatas, 0); //agarro la primera
	for(i = 1; i < particionesCandidatas->elements_count; i++){ //comparo con la que sigue
		t_particion* particionActual = list_get(particionesCandidatas, i);
		if(particionAEliminar->id > particionActual->id){
			particionAEliminar = particionActual;
		}
	}
	list_destroy(particionesCandidatas);
	return particionAEliminar;
}

int obtenerPosicion(t_particion * particion){
	int i;
	for(i=0; i < tabla_particiones->elements_count; i++){
		t_particion*particionAMirar = list_get(tabla_particiones, i);
		if(particion->id == particionAMirar->id && particion->posicion_inicial == particionAMirar->posicion_inicial){
			return i;
		}
	}
	printf("No encontre el elemento de id %d con posicion inicial %d \n", particion->id, particion->posicion_inicial);
	return -1;
}

void actualizarID(int32_t id_mensaje){
	int i;
	for(i=0; i < tabla_particiones->elements_count; i++){
		t_particion*particionAMirar = list_get(tabla_particiones, i);
		if(particionAMirar->id_mensaje == id_mensaje){
			particionAMirar->id = get_id_particion();
		}
	}
}

void guardarMensaje(info_mensaje * mensaje, t_particion * particion){
	particion->codigo_operacion = mensaje->op_code;
	particion->ocupada = true;
	particion->id_mensaje = mensaje->id_mensaje;
	particion->id = get_id_particion();
	log_info(logger, "Se guardo un mensaje en la memoria. Posicion de inicio: %d", inicioMemoria + particion->posicion_inicial);
}

int32_t tamanioMinimo(int32_t sizeMsg){
///algoritmo para calcular la menor potencia de 2 en la que entra un mensaje; Buddy System
	if(sizeMsg !=1){
		int32_t menorPotenciaDeDos=2;
		while(menorPotenciaDeDos < sizeMsg){
			menorPotenciaDeDos = menorPotenciaDeDos*2;
		}
		return menorPotenciaDeDos;
	} else return 1;
}

void generarParticionBS(t_particion* particionInicial){
	int halfSize= particionInicial->size * 0.5;

	int inicioIzquierda = particionInicial->posicion_inicial;
	int inicioDerecha = particionInicial->posicion_inicial+((particionInicial->size)*0.5);

	t_particion *particionIzquierda = crearParticion(inicioIzquierda, halfSize, false);
	particionIzquierda->id = get_id_particion();
	sleep(0.25);
	t_particion *particionDerecha = crearParticion(inicioDerecha, halfSize, false);
	particionDerecha->id = get_id_particion();

	int posicion = obtenerPosicion(particionInicial);
	list_remove_and_destroy_element(tabla_particiones, posicion, free);
	list_add_in_index(tabla_particiones,posicion, particionDerecha);
	list_add_in_index(tabla_particiones,posicion, particionIzquierda);
}

// En Buddy System una particion es candidata si su tamaño es el de la menor potencia de dos
bool particionCandidataBS(t_particion* particion, int32_t tamanioMinimo){
	return !particion->ocupada && tamanioMinimo == particion->size;
}

bool hayParticionesCandidatasBS(int32_t sizeMsg){
	bool _particionCandidataBS(void* element){
		return particionCandidataBS((t_particion*)element, sizeMsg);
	}
	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidataBS);
	bool resultado = particionesCandidatas->elements_count != 0;
	list_destroy(particionesCandidatas);
	return resultado;
}

t_particion* getParticionBS(int32_t tamanioMinimo){
	bool _particionCandidataBS(void* element){
		return particionCandidata((t_particion*)element, tamanioMinimo);
	}
	t_list* particionesCandidatas = list_filter(tabla_particiones, _particionCandidataBS);
	t_particion* candidato = list_get(particionesCandidatas, 0);
	list_destroy(particionesCandidatas);
	return candidato;
}

void algoritmoBuddySystem(info_mensaje * mensaje, int32_t algoritmoReemplazo){
	int32_t tamanio= tamanioMinimo(mensaje->sizeMsg);
	if(tamanio < sizeMinParticion){
		tamanio = sizeMinParticion;
	}
	t_particion * particion;
	bool buscar = true;
	int32_t liberadas = 0;
	int32_t cantElementos = tabla_particiones->elements_count;

	while(buscar){
		if(liberadas <= cantElementos){
			if(hayParticionesCandidatasBS(tamanio) == true){ //hay una particion libre del tamanio exacto que necesito?
				particion = getParticionBS(tamanio);
				guardarMensaje(mensaje, particion);
				buscar = false;
			}else {if(hayParticionesCandidatas(tamanio) == true){ //hay una particion libre que pueda truncar?
				while(hayParticionesCandidatasBS(tamanio) != true){
					particion = getParticionFirstFit(tamanio);
					generarParticionBS(particion);
					}
				particion = getParticionBS(tamanio);
				guardarMensaje(mensaje, particion);
				buscar= false;
				}else {
					algoritmoLiberacionBS(algoritmoReemplazo);
					liberadas++;
				}
			}
		}else {
			buscar=false;
			printf("Libere todas las particiones y no encontre un lugar para guardar el mensaje\n");
		}
	}
}

void algoritmoParticionDinamica(info_mensaje * mensaje, int32_t frecuenciaCompactacion, int32_t algoritmoReemplazo, int32_t algoritmoParticionLibre){
	int32_t tamanio= mensaje->sizeMsg;
	if(tamanio < sizeMinParticion){
		tamanio = sizeMinParticion;
	}
	t_particion * particion;
	bool buscar = true;
	int32_t liberadas = 0;
	int32_t cantElementos = tabla_particiones->elements_count;

	while(buscar){
		if(liberadas <= cantElementos){
			if(hayParticionesCandidatas(tamanio) == true){ //hay una particion libre que pueda truncar?
				switch(algoritmoParticionLibre){
				case FF:
					particion = getParticionFirstFit(tamanio);
					if(particion->size == tamanio){
						guardarMensaje(mensaje, particion);
						buscar = false;
					}else {
						generarParticionDinamica(particion, tamanio);
						particion = getParticionFirstFit(tamanio);
						guardarMensaje(mensaje, particion);
						buscar=false;
					}
					break;
				case BF:
					particion = getParticionBestFit(tamanio);
					if(particion->size == tamanio){
						guardarMensaje(mensaje, particion);
						buscar=false;
					}else{
						generarParticionDinamica(particion, tamanio);
						particion = getParticionBestFit(tamanio);
						guardarMensaje(mensaje, particion);
						buscar=false;
					}
					break;
				default:
					printf("Ese algoritmo no esta implementado\n");
					return;
				}
			}else {
				algoritmoLiberacion(algoritmoReemplazo);
				liberadas++;
				switch(frecuenciaCompactacion){
				case -1:
					break;
				case 0 || 1:
					algoritmoCompactacion();
					break;
				default:
					if(liberadas == frecuenciaCompactacion){
						algoritmoCompactacion();
					}
					break;
				}
			}
		} else{
			buscar=false;
			printf("Libere todas las particiones y no encontre un lugar para guardar el mensaje\n");
		}
	}
}

void mostrarEstadoMemoria(){
	for(int i = 0; i<tabla_particiones->elements_count;i++){
		t_particion * particion = list_get(tabla_particiones, i);
		char estado[] = "L";
		if(particion->ocupada == 1) estado[0] = 'X';
		printf("Inicio: %d. Final: %d. Estado: [%s]\n", particion->posicion_inicial, particion->posicion_final, estado);
	}
}





