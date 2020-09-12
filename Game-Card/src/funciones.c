#include "funciones.h"

void funcion_new_pokemon(void* new_y_id) {

	t_New* new = NULL;
	memcpy(&new, new_y_id, sizeof(t_New*));

	log_info(logger_GC, "Iniciando operacion NEW_POKEMON para %s...", new->pokemon.nombre);

	char* ruta_metadata = ruta_metadata_pokemon_teorica (new->pokemon);

	sem_wait (&diccionario);
	if (existe (ruta_metadata)) {
		sem_post (&diccionario);

		sem_wait (dictionary_get(semaforos, new->pokemon.nombre));
		while (get_open (ruta_metadata) == 'Y') {
			sem_post (dictionary_get(semaforos, new->pokemon.nombre));
			sleep (tiempo_reintento_operacion);
			sem_wait (dictionary_get(semaforos, new->pokemon.nombre));
		}
		set_open (ruta_metadata, 'Y');
		FILE* metadata = abrir_para (ruta_metadata, "r");
		char* linea_con_bloques = metadata_copiar_linea_bloques(metadata, ruta_metadata);
		fclose(metadata);
		sem_post (dictionary_get(semaforos, new->pokemon.nombre));

		char** linea_dividida = string_split(linea_con_bloques, "=");

		int32_t cantidad = cantidad_de_bloques (linea_dividida[1]);

		if (cantidad != 0) {

			char** strings_bloques = string_get_string_as_array(linea_dividida[1]);

			char* archivo_cargado = traer_bloques(strings_bloques, cantidad);

			liberar_bloques (strings_bloques, cantidad, ruta_metadata, new->pokemon.nombre);

			char* apuntador = apuntar_a_posicion(archivo_cargado, new->posicion);
			if (apuntador != NULL) archivo_cargado = agregar_cantidad(archivo_cargado, apuntador, new->cant, new->posicion);
			else archivo_cargado = agregar_nueva_posicion(archivo_cargado, new->posicion, new->cant);

			volcar_archivo_cargado(archivo_cargado, ruta_metadata, new->pokemon.nombre);

			string_iterate_lines(strings_bloques, (void*) free);
			free(strings_bloques);
		}
		else {
			log_info (logger_GC, "Creando al pokemon %s...", new->pokemon.nombre);

			char* pos_nueva = posicion_a_string(new->posicion);
			char* cant_nueva = string_itoa(new->cant);
			char* archivo_nuevo = malloc (strlen(pos_nueva) + strlen(cant_nueva) + 2); //Más 3 por el '=' y el '\0'
			strcat (strcat (strcpy(archivo_nuevo, pos_nueva), "="), cant_nueva);
			volcar_archivo_cargado(archivo_nuevo, ruta_metadata, new->pokemon.nombre);

			free(pos_nueva);
			free(cant_nueva);
		}

		sleep(tiempo_retardo_operacion);

		sem_wait (dictionary_get(semaforos, new->pokemon.nombre));
		set_open (ruta_metadata, 'N');
		sem_post (dictionary_get(semaforos, new->pokemon.nombre));

		free(linea_con_bloques);
		liberar_strings(linea_dividida);

	}
	else {
		sem_t* nuevo_semaforo = malloc(sizeof(sem_t));
		dictionary_put (semaforos, new->pokemon.nombre, nuevo_semaforo);
		sem_init (dictionary_get (semaforos, new->pokemon.nombre), 0, 1);
		crear_metadata_new (new, ruta_metadata);
		sem_post (&diccionario);

		log_info (logger_GC, "Creando al pokemon %s...", new->pokemon.nombre);

		char* posicion_nueva = posicion_a_string(new->posicion);
		char* cantidad_nueva = string_itoa(new->cant);
		char* archivo_nuevo = malloc (strlen(posicion_nueva) + strlen(cantidad_nueva) + 2); //Más 3 por el '=' y el '\0'
		strcat (strcat (strcpy(archivo_nuevo, posicion_nueva), "="), cantidad_nueva);
		volcar_archivo_cargado(archivo_nuevo, ruta_metadata, new->pokemon.nombre);

		sleep (tiempo_retardo_operacion);

		log_info (logger_GC, "Listo, pokemon %s creado.", new->pokemon.nombre);

		sem_wait(dictionary_get(semaforos, new->pokemon.nombre));
		set_open (ruta_metadata, 'N');
		sem_post(dictionary_get(semaforos, new->pokemon.nombre));

		free(posicion_nueva);
		free(cantidad_nueva);
	}

	int32_t id_mensaje = 0;
	memcpy(&id_mensaje, new_y_id + sizeof(t_New*), sizeof(id_mensaje));

	char* id_msj = string_itoa(id_mensaje);
	char* posX = string_itoa(new->posicion.X);
	char* posY = string_itoa(new->posicion.Y);

	enviar_appeared(new->pokemon.nombre, posX, posY, id_msj);

	free(ruta_metadata);
	free(id_msj);
	free(posX);
	free(posY);

	free(new->pokemon.nombre);
	free(new);
	free(new_y_id);
}

void funcion_catch_pokemon(void* catch_y_id) {
	t_Catch* catch = NULL;
	memcpy(&catch, catch_y_id, sizeof(t_Catch*));

	int32_t id_mensaje = 0;
	memcpy(&id_mensaje, catch_y_id + sizeof(t_Catch*), sizeof(id_mensaje));

	char* id_msj = string_itoa(id_mensaje);

	log_info(logger_GC, "Iniciando operacion CATCH para %s...", catch->pokemon.nombre);

	char* ruta_metadata = ruta_metadata_pokemon_teorica (catch->pokemon);

	if (existe (ruta_metadata)) {

		sem_wait (dictionary_get(semaforos, catch->pokemon.nombre));
		while (get_open (ruta_metadata) == 'Y') {
			sem_post (dictionary_get(semaforos, catch->pokemon.nombre));
			sleep (tiempo_reintento_operacion);
			sem_wait (dictionary_get(semaforos, catch->pokemon.nombre));
		}
		set_open (ruta_metadata, 'Y');
		FILE* metadata = abrir_para (ruta_metadata, "r");
		char* linea_con_bloques = metadata_copiar_linea_bloques(metadata, ruta_metadata);
		fclose(metadata);
		sem_post (dictionary_get(semaforos, catch->pokemon.nombre));

		char** linea_dividida = string_split(linea_con_bloques, "=");

		int32_t cantidad = cantidad_de_bloques (linea_dividida[1]);

		if (cantidad == 0) {
			log_info (logger_GC, "En este momento no hay ejemplares de %ss en el mapa.", catch->pokemon.nombre);
			enviar_caught (id_msj, "FAIL");

			sem_wait (dictionary_get(semaforos, catch->pokemon.nombre));
			set_open (ruta_metadata, 'N');
			sem_post (dictionary_get(semaforos, catch->pokemon.nombre));
		}

		else {
			char** strings_bloques = string_get_string_as_array(linea_dividida[1]);

			char* archivo_cargado = traer_bloques(strings_bloques, cantidad);

			liberar_bloques (strings_bloques, cantidad, ruta_metadata, catch->pokemon.nombre);

			char* apuntador = apuntar_a_posicion(archivo_cargado, catch->posicion);

			if (apuntador == NULL)
				log_info (logger_GC, "No hay ningun %s en la posicion (%d, %d).", catch->pokemon.nombre, catch->posicion.X, catch->posicion.Y);

			else archivo_cargado = quitar_pokemon(archivo_cargado, apuntador, catch->posicion);

			volcar_archivo_cargado(archivo_cargado, ruta_metadata, catch->pokemon.nombre);

			sleep(tiempo_retardo_operacion);

			sem_wait (dictionary_get(semaforos, catch->pokemon.nombre));
			set_open (ruta_metadata, 'N');
			sem_post (dictionary_get(semaforos, catch->pokemon.nombre));

			if (apuntador == NULL) enviar_caught (id_msj, "FAIL");

			else enviar_caught (id_msj, "OK");

			string_iterate_lines(strings_bloques, (void*) free);
			free(strings_bloques);
		}

		free(linea_con_bloques);
		liberar_strings(linea_dividida);
	}
	else {
		log_info (logger_GC, "En este momento no hay ejemplares de %ss en el mapa.", catch->pokemon.nombre);
		enviar_caught (id_msj, "FAIL");
	}

	free(ruta_metadata);
	free(id_msj);

	free(catch->pokemon.nombre);
	free(catch);
	free(catch_y_id);
}

void funcion_get_pokemon(void* get_y_id) {
	t_Get* get = NULL;
	memcpy(&get, get_y_id, sizeof(t_Get*));

	int32_t id_mensaje = 0;
	memcpy(&id_mensaje, get_y_id + sizeof(t_Get*), sizeof(id_mensaje));

	log_info(logger_GC, "Iniciando operacion GET para %s...", get->pokemon.nombre);

	char* ruta_metadata = ruta_metadata_pokemon_teorica (get->pokemon);


	if (existe (ruta_metadata)) {

		sem_wait (dictionary_get(semaforos, get->pokemon.nombre));
		while (get_open (ruta_metadata) == 'Y') {
			sem_post (dictionary_get(semaforos, get->pokemon.nombre));
			sleep (tiempo_reintento_operacion);
			sem_wait (dictionary_get(semaforos, get->pokemon.nombre));
		}
		set_open (ruta_metadata, 'Y');
		FILE* metadata = abrir_para (ruta_metadata, "r");
		char* linea_con_bloques = metadata_copiar_linea_bloques(metadata, ruta_metadata);
		fclose(metadata);
		sem_post (dictionary_get(semaforos, get->pokemon.nombre));

		char** linea_dividida = string_split(linea_con_bloques, "=");

		int32_t cantidad = cantidad_de_bloques (linea_dividida[1]);

		if (cantidad == 0) {
			log_info (logger_GC, "En este momento no hay ejemplares de %ss en el mapa.", get->pokemon.nombre);
			t_list * listaNull = list_create();
			enviar_localized(listaNull, get->pokemon, id_mensaje);
			list_destroy(listaNull);

			sem_wait (dictionary_get(semaforos, get->pokemon.nombre));
			set_open (ruta_metadata, 'N');
			sem_post (dictionary_get(semaforos, get->pokemon.nombre));
		}

		else {
			char** strings_bloques = string_get_string_as_array(linea_dividida[1]);

			char* archivo_cargado = traer_bloques(strings_bloques, cantidad);

			char** lineas_archivo = string_split(archivo_cargado, "\n");

			free(archivo_cargado);

			char* posiciones = NULL;

			int32_t i = 0, j = 0, c = 0;
			char aux;

			while (lineas_archivo[i] != NULL) {
				if (lineas_archivo[i][j] == '=') {
					aux = ' ';
					posiciones = realloc(posiciones, c + 1);
					posiciones[c] = aux;
					j = 0;
					i++, c++;
					continue;
				}

				if (lineas_archivo[i][j] == '-') {
					aux = ' ';
					posiciones = realloc(posiciones, c + 1);
					posiciones[c] = aux;
					j++; c++;
				}

				else {
					aux = lineas_archivo[i][j];
					posiciones = realloc(posiciones, c + 1);
					posiciones[c] = aux;
					j++; c++;
				}
			}

			posiciones[c-1] = '\0';

			char** posiciones_separadas = string_split(posiciones, " ");

			free(posiciones);

			t_list* posiciones_a_enviar = list_create();
			i = 0;
			t_posicion* auxiliar = NULL;

			while(posiciones_separadas[i] != NULL) {
				auxiliar = malloc (sizeof(t_posicion));
				auxiliar->X = atoi (posiciones_separadas[i]);
				i++;
				auxiliar->Y = atoi (posiciones_separadas[i]);
				i++;
				list_add(posiciones_a_enviar, auxiliar);
			}

			sleep(tiempo_retardo_operacion);

			sem_wait (dictionary_get(semaforos, get->pokemon.nombre));
			set_open (ruta_metadata, 'N');
			sem_post (dictionary_get(semaforos, get->pokemon.nombre));


			enviar_localized(posiciones_a_enviar, get->pokemon, id_mensaje);

			liberar_strings(lineas_archivo);
			liberar_strings(posiciones_separadas);
			string_iterate_lines(strings_bloques, (void*) free);
			free(strings_bloques);

			list_destroy_and_destroy_elements(posiciones_a_enviar, free);
		}

		liberar_strings(linea_dividida);
		free(linea_con_bloques);
	}

	else {
		log_info (logger_GC, "En este momento no hay ejemplares de %ss en el mapa.", get->pokemon.nombre);
		t_list * listaNull = list_create();
		enviar_localized(listaNull, get->pokemon, id_mensaje);
		list_destroy(listaNull);
	}

	free(ruta_metadata);

	free(get->pokemon.nombre);
	free(get);
	free(get_y_id);
}

/****************************************************************************/
/**********************************AUXILIARES********************************/
/****************************************************************************/

int32_t existe (char* ruta) {
	struct stat estado_archivo;
	return (stat (ruta, &estado_archivo) == 0);
}

void crear_metadata_new (t_New* new, char* ruta) {
	char* carpeta_metadata = ruta_carpeta_pokemon_teorica(new->pokemon);
	if (mkdir (carpeta_metadata, S_IRWXU | S_IROTH) != 0) salir("Error al crear directorio para metadata del nuevo pokemon.");

	FILE* ruta_metadata_new = abrir_para (ruta, "w+");

	fprintf (ruta_metadata_new, "DIRECTORY=N\n");
	fprintf (ruta_metadata_new, "SIZE=0\n");
	fprintf (ruta_metadata_new, "BLOCKS=[]\n");
	fprintf (ruta_metadata_new, "OPEN=Y");

	fclose(ruta_metadata_new);
	free(carpeta_metadata);

	log_info (logger_GC, "Archivo para metadatos del pokemon %s creado.", new->pokemon.nombre);
}

off_t asignar_bloque_libre() {
	off_t indice = 0;
	sem_wait(&bitmap);
	while(bitarray_test_bit(&mapa_de_bloques, indice)) indice++;
	bitarray_set_bit(&mapa_de_bloques, indice);
	actualizar_bitmap();
	sem_post(&bitmap);

	log_info (logger_GC, "Se le ha asignado el bloque %ld.", indice);

	return indice;
}

char get_open (char* ruta_metadata) {

	FILE* metadata = abrir_para (ruta_metadata, "r");

	struct stat estado_archivo;
	stat(ruta_metadata, &estado_archivo);

	char* stream = malloc(estado_archivo.st_size + 1);

	fread(stream, estado_archivo.st_size, 1, metadata);

	fclose (metadata);

	stream[estado_archivo.st_size] = '\0';

	char** lineas = string_split(stream, "\n");
	free(stream);

	if (string_contains(lineas[3], "OPEN=Y")) {
		liberar_strings(lineas);
		return 'Y';
	}
	liberar_strings(lineas);
	return 'N';
}

void set_open (char* ruta_metadata, char valor) {

	FILE* metadata = abrir_para (ruta_metadata, "r");

	struct stat estado_archivo;
	stat(ruta_metadata, &estado_archivo);

	char* stream = malloc(estado_archivo.st_size + 1);

	fread(stream, estado_archivo.st_size, 1, metadata);

	fclose (metadata);

	stream[estado_archivo.st_size] = '\0';

	metadata = abrir_para (ruta_metadata, "w");

	char** lineas = string_split(stream, "\n");
	free(stream);

	lineas[3][5] = valor;

	int i = 0;
	while(i < 3) {
		fputs(lineas[i], metadata);
		fputc('\n', metadata);
		i++;
	}
	fputs(lineas[i], metadata);

	fclose (metadata);

	liberar_strings(lineas);

	log_info (logger_GC, "Seteado OPEN=%c en metadata del pokemon.", valor);
}

void actualizar_bitmap () {
	char ruta_archivo_Metadata_Bitmap[tam_punto_de_montaje + strlen ("/Metadata/Bitmap.bin") + 1];
	strcat(strcpy(ruta_archivo_Metadata_Bitmap, punto_de_montaje), "/Metadata/Bitmap.bin");
	FILE* bitmap = fopen (ruta_archivo_Metadata_Bitmap, "w+");
	fwrite (mapa_de_bloques.bitarray, sizeof(char), mapa_de_bloques.size, bitmap);
	fclose(bitmap);
}

int32_t cantidad_de_bloques (char* vector){
	int32_t i = 1, comas = 0;
	while (vector[i] != ']') {
		if (vector[i] == ',') comas++;
		i++;
	}
	if (i == 1 || i == 2) return (i-1);

	log_info (logger_GC, "Cantidad de bloques que lo conforman: %d.", comas+1);

	return comas+1;
}

void liberar_bloques (char** strings_bloques, int32_t cantidad, char* ruta_metadata, char* pokemon){
	int32_t indice = 0;
	sem_wait(&bitmap);
	while(indice < cantidad) {
		bitarray_clean_bit(&mapa_de_bloques, atol(strings_bloques[indice]));
		indice++;
	}
	actualizar_bitmap();
	sem_post(&bitmap);

	int32_t tam_archivo_cargado;

	char* archivo_cargado = metadata_traer(ruta_metadata, pokemon, &tam_archivo_cargado);

	int32_t i = 0, j = 0;
	while (archivo_cargado[i] != ']') i++;
	i--;

	if (archivo_cargado[i] == '[') free(archivo_cargado);

	else {
		while (archivo_cargado[i-j] != '[') j++;

		memmove(archivo_cargado + i - j + 1, archivo_cargado + i + 1, strlen(archivo_cargado + i + 1) + 1);
		archivo_cargado = realloc(archivo_cargado, tam_archivo_cargado - j + 1);

		metadata_volcar (ruta_metadata, pokemon, archivo_cargado);

		free(archivo_cargado);
	}

	log_info (logger_GC, "Bloques del archivo liberados.");
}

char* apuntar_a_posicion (char* archivo_cargado, t_posicion posicion) {
	char* dupla = posicion_a_string (posicion);
	char* puntero_substring = strstr(archivo_cargado, dupla);
	free(dupla);
	return puntero_substring;
}

char* agregar_cantidad(char* archivo_cargado, char* apuntador, int32_t cantidad_nueva, t_posicion posicion){
	int32_t i = 0;
	int32_t tam = 0;
	char* pos_cantidad_actual;

	while (apuntador[i] != '=' && apuntador[i] != '\0') i++;
	if(apuntador[i] == '\0') salir("Error en agregar_cantidad");

	i++;
	pos_cantidad_actual = apuntador + i;

	while (apuntador[i] != '\n' && apuntador[i] != '\0') {tam++; i++;}

	char* str_cantidad_actual = malloc (tam+1);
	strncpy(str_cantidad_actual, pos_cantidad_actual, tam);
	str_cantidad_actual[tam] = '\0';

	int32_t cantidad_actual = (int32_t) atoi (str_cantidad_actual);
	int32_t cantidad_total = cantidad_actual + cantidad_nueva;

	char* str_cantidad_total = string_itoa(cantidad_total);
	int32_t tam_nuevo = strlen(str_cantidad_total);

	if (tam_nuevo == tam) strncpy(pos_cantidad_actual, str_cantidad_total, tam);

	else {
		i = 0;
		int32_t diferencia = tam_nuevo - tam;

		archivo_cargado = realloc(archivo_cargado, strlen(archivo_cargado) + diferencia + 1);

		pos_cantidad_actual = apuntar_a_posicion(archivo_cargado, posicion);
		while (pos_cantidad_actual[i] != '=') i++;
		pos_cantidad_actual = pos_cantidad_actual + i + 1;

		memmove(pos_cantidad_actual + tam_nuevo, pos_cantidad_actual + tam, strlen(pos_cantidad_actual + tam) + 1);

		memcpy(pos_cantidad_actual, str_cantidad_total, tam_nuevo);
	}

	free(str_cantidad_actual);
	free(str_cantidad_total);

	log_info (logger_GC, "Ya se encontraba este pokemon en la posicion especificada.");
	log_info (logger_GC, "Han sido agregados %d nuevos.", cantidad_nueva);

	return archivo_cargado;
}

char* agregar_nueva_posicion(char* archivo_cargado, t_posicion posicion_nueva, int32_t cantidad_nueva) {

	char* str_posicion_nueva = posicion_a_string(posicion_nueva);

	char* str_cantidad_nueva = string_itoa(cantidad_nueva);

	char* nueva_linea = malloc(strlen(str_posicion_nueva) + strlen(str_cantidad_nueva) + 3); // +3 Para '=', '\n' y '\0'
	strcat (strcat (strcat (strcpy(nueva_linea, "\n"), str_posicion_nueva), "="), str_cantidad_nueva);

	int32_t ultima_posicion = strlen(archivo_cargado);
	int32_t len_nueva_linea = strlen(nueva_linea);
	archivo_cargado = realloc(archivo_cargado, ultima_posicion + strlen(nueva_linea) + 1);

	memmove(archivo_cargado + ultima_posicion, nueva_linea, len_nueva_linea + 1);

	free(str_posicion_nueva);
	free(nueva_linea);
	free(str_cantidad_nueva);

	log_info (logger_GC, "Posicion sin ejemplares de este pokemon.");
	log_info (logger_GC, "Han sido agregados %d nuevos.", cantidad_nueva);

	return archivo_cargado;
}

void volcar_archivo_cargado(char* archivo_cargado, char* ruta_metadata, char* pokemon) {

	if (archivo_cargado == NULL) {
		log_info (logger_GC, "El archivo quedo vacio. Recursos liberados.");
		metadata_actualizar_size (ruta_metadata, pokemon, 0);
		return;
	}

	int32_t tam_total = strlen(archivo_cargado) + 1; // +1 por el EOF = #

	archivo_cargado = realloc(archivo_cargado, tam_total + 1); // +1 para '\0'
	archivo_cargado[tam_total-1] = '#';
	archivo_cargado[tam_total] = '\0';

	int32_t bloques_necesarios;
	FILE* auxiliar = NULL;

	off_t bloque_libre;
	char* str_bloque_libre;
	char* ruta_dinamica;

	if (tam_total % tamanio_bloque == 0) bloques_necesarios = tam_total/tamanio_bloque;
	else bloques_necesarios = tam_total/tamanio_bloque + 1;

	int32_t resta = tam_total;
	int32_t i;

	for (i = 0; i < bloques_necesarios; i++){
		bloque_libre = asignar_bloque_libre();
		str_bloque_libre = string_itoa (bloque_libre);
		metadata_agregar_bloque (ruta_metadata, pokemon, str_bloque_libre);

		ruta_dinamica = malloc(strlen (punto_de_montaje) + strlen("/Blocks/.bin") + strlen(str_bloque_libre) + 1);
		strcat (strcat (strcat (strcpy(ruta_dinamica, punto_de_montaje), "/Blocks/"), str_bloque_libre), ".bin");

		if (resta >= tamanio_bloque){

			auxiliar = abrir_para (ruta_dinamica, "w+");
			fwrite (archivo_cargado+(i*tamanio_bloque), sizeof(char), tamanio_bloque, auxiliar);
			fclose (auxiliar);

			resta -= tamanio_bloque;

			free(ruta_dinamica);
			free(str_bloque_libre);
		}
		else {
			auxiliar = abrir_para (ruta_dinamica, "w+");
			fwrite (archivo_cargado+(i*tamanio_bloque), sizeof(char), resta, auxiliar);
			fclose (auxiliar);

			free(ruta_dinamica);
			free(str_bloque_libre);
		}
	}

	metadata_actualizar_size (ruta_metadata, pokemon, tam_total);

	free(archivo_cargado);
}

void metadata_agregar_bloque (char* ruta_metadata, char* pokemon, char* bloque) {

	int32_t tam_archivo_cargado;

	char* archivo_cargado = metadata_traer(ruta_metadata, pokemon, &tam_archivo_cargado);

	int32_t i = 0;
	while (*(archivo_cargado+i) != ']') i++;

	if (*(archivo_cargado+i-1) == '[') {
		archivo_cargado = realloc(archivo_cargado, tam_archivo_cargado + strlen(bloque) + 1);
		memmove(archivo_cargado + i + strlen(bloque), archivo_cargado + i, strlen(archivo_cargado + i) + 1);
		memcpy(archivo_cargado + i, bloque, strlen(bloque));
	}
	else {
		char* aux = malloc(strlen(bloque)+2);
		strcat (strcpy(aux, ","), bloque);

		archivo_cargado = realloc(archivo_cargado, tam_archivo_cargado + strlen(aux) + 1);
		memmove(archivo_cargado + i + strlen(aux), archivo_cargado + i, strlen(archivo_cargado + i) + 1);
		memcpy(archivo_cargado + i, aux, strlen(aux));

		free(aux);
	}

	metadata_volcar (ruta_metadata, pokemon, archivo_cargado);

	free(archivo_cargado);

	log_info (logger_GC, "Bloques en metadata actualizados.");
}

void metadata_actualizar_size (char* ruta_metadata, char* pokemon, int32_t size_nuevo) {

	int32_t tam_archivo_cargado;

	char* archivo_cargado = metadata_traer(ruta_metadata, pokemon, &tam_archivo_cargado);

	char* linea_size = strstr(archivo_cargado, "SIZE=");

	char* str_size_nuevo = string_itoa (size_nuevo);

	int32_t i = 5;
	while (linea_size[i] != '\n') i++;

	if(strlen(str_size_nuevo) == (i-5)) memcpy(linea_size + 5, str_size_nuevo, strlen(str_size_nuevo));

	else {
		archivo_cargado = realloc(archivo_cargado, tam_archivo_cargado + strlen(str_size_nuevo) + 1);
		linea_size = strstr(archivo_cargado, "SIZE=");
		memmove(linea_size + 5 + strlen(str_size_nuevo), linea_size + i, strlen(linea_size + i) + 1);
		memcpy(linea_size + 5, str_size_nuevo, strlen(str_size_nuevo));
	}

	metadata_volcar (ruta_metadata, pokemon, archivo_cargado);

	free(archivo_cargado);
	free(str_size_nuevo);

	log_info (logger_GC, "Tamanio del archivo actualizado.");
}

void metadata_volcar (char* ruta_metadata, char* pokemon, char* archivo_cargado) {
	sem_wait (dictionary_get(semaforos, pokemon));
	FILE* metadata = abrir_para (ruta_metadata, "w+");
	fwrite(archivo_cargado, strlen(archivo_cargado) + 1, 1, metadata);
	fclose(metadata);
	sem_post (dictionary_get(semaforos, pokemon));
}

void liberar_strings(char** cadenas) {
	int i = 0;
	while(cadenas[i] != NULL) {
	        free(cadenas[i]);
	        i++;
	}
	free(cadenas);
}

char* quitar_pokemon (char* archivo_cargado, char* apuntador, t_posicion posicion) {
	int32_t i = 0;
	int32_t tam = 0;
	char* pos_cantidad_actual;

	while (apuntador[i] != '=' && apuntador[i] != '\0') i++;
	if(apuntador[i] == '\0') salir("Error en quitar_pokemon");

	i++;
	pos_cantidad_actual = apuntador + i;

	while (apuntador[i] != '\n' && apuntador[i] != '\0') {tam++; i++;}

	char* str_cantidad_actual = malloc (tam+1);
	strncpy(str_cantidad_actual, pos_cantidad_actual, tam);
	str_cantidad_actual[tam] = '\0';

	int32_t cantidad_actual = (int32_t) atoi (str_cantidad_actual);
	int32_t cantidad_total = cantidad_actual - 1;

	char* str_cantidad_total = string_itoa(cantidad_total);
	int32_t tam_nuevo = strlen(str_cantidad_total);

	if (tam_nuevo == tam && cantidad_total != 0) strncpy(pos_cantidad_actual, str_cantidad_total, tam);

	if (tam_nuevo != tam && cantidad_total != 0) {

		memmove(apuntador + i - 1, apuntador + i, strlen(apuntador + i) + 1);
		strncpy(pos_cantidad_actual, str_cantidad_total, tam_nuevo);
		archivo_cargado = realloc(archivo_cargado, strlen(archivo_cargado) + 1);
	}

	if (cantidad_total == 0) {
		if(apuntador != archivo_cargado) {
			memmove(apuntador - 1, apuntador + i, strlen(apuntador + i) + 1);
			archivo_cargado = realloc(archivo_cargado, strlen(archivo_cargado) + 1);
		}
		else {
			if (*(apuntador + i) == '\0') {
				free (archivo_cargado);
				archivo_cargado = NULL;
			}
			else {
				memmove(apuntador, apuntador + i + 1, strlen(apuntador + i + 1) + 1);
				archivo_cargado = realloc(archivo_cargado, strlen(archivo_cargado) + 1);
			}
		}
	}

	free(str_cantidad_actual);
	free(str_cantidad_total);

	log_info (logger_GC, "Se produjo un catch en la posicion (%d, %d).", posicion.X, posicion.Y);
	log_info (logger_GC, "Se elimino un ejemplar de este pokemon");

	return archivo_cargado;
}

/*¡¡¡Se debe liberar memoria luego de usarlas!!!*/
char* ruta_carpeta_pokemon_teorica (t_pokemon pokemon) {

	char* ruta = malloc (tam_punto_de_montaje + strlen ("/Files/") + pokemon.size_Nombre);

	return strcat (strcat (strcpy(ruta, punto_de_montaje), "/Files/"), pokemon.nombre);
}

char* ruta_metadata_pokemon_teorica (t_pokemon pokemon) {

	char* ruta = malloc (tam_punto_de_montaje + strlen("/Files//Metadata.bin") + pokemon.size_Nombre);

	return strcat (strcat (strcat (strcpy(ruta, punto_de_montaje), "/Files/"), pokemon.nombre), "/Metadata.bin");
}

FILE* abrir_para (char* ruta, char* modo) {
	FILE* archivo = fopen(ruta, modo);
	if (archivo == NULL) salir("Fallo de fopen");
	return archivo;
}

char* copiar_stream(FILE* archivo_lectura, char* ruta) {

	struct stat estado_archivo;
	stat(ruta, &estado_archivo);

	char* stream = malloc(estado_archivo.st_size);

	fread(stream, estado_archivo.st_size, 1, archivo_lectura);

	if (stream[estado_archivo.st_size - 1] == '#') stream = realloc(stream, estado_archivo.st_size - 1);

	return stream;
}

char* metadata_copiar_linea_bloques(FILE* metadata, char* ruta_archivo) {

	char aux;
	int32_t contador_de_lineas = 0, i = 0;

	while (contador_de_lineas < 2) {
		aux = fgetc (metadata);
		if (aux == '\n') contador_de_lineas++;
	}

	long int posicion = ftell (metadata);

	aux = '\0';

	while (aux != '\n') {
		aux = fgetc (metadata);
		i++;
	}

	fseek(metadata, posicion, SEEK_SET);

	char* linea_bloques = malloc (i);
	fread(linea_bloques, i-1, 1, metadata);
	linea_bloques[i-1] = '\0';

	return linea_bloques;
}

char* traer_bloques(char** bloques, int32_t cantidad) {

	if (cantidad == 0) return NULL;

	char carpeta_Blocks[tam_punto_de_montaje + strlen ("/Blocks") + 1];
	strcat(strcpy(carpeta_Blocks, punto_de_montaje), "/Blocks");

	int32_t i = 0;
	struct stat estado_binario;
	char* carpeta_dinamica = NULL;
	char* datos_copiados = NULL;
	char* auxiliar = NULL;
	FILE* binario = NULL;
	int32_t tamanio_total = 0, tamanio_previo = 0, tamanio_auxiliar = 0;

	while (i < cantidad) {
		carpeta_dinamica = malloc(strlen (carpeta_Blocks) + strlen("/.bin") + strlen(bloques[i]) + 1);
		strcat (strcat (strcat (strcpy(carpeta_dinamica, punto_de_montaje), "/Blocks/"), bloques[i]), ".bin");
		stat(carpeta_dinamica, &estado_binario);

		tamanio_previo = tamanio_total;

		binario = abrir_para (carpeta_dinamica, "r");
		auxiliar = copiar_stream_con_tamanio(binario, carpeta_dinamica, &tamanio_auxiliar);
		fclose(binario);

		tamanio_total += tamanio_auxiliar;

		datos_copiados = realloc (datos_copiados, tamanio_total);

		memcpy(datos_copiados+tamanio_previo, auxiliar, tamanio_auxiliar);

		free(carpeta_dinamica);
		free(auxiliar);
		i++;
	}
	datos_copiados = realloc (datos_copiados, tamanio_total+1);
	datos_copiados[tamanio_total] = '\0';

	log_info (logger_GC, "El archivo ha sido cargado a memoria.");

	return datos_copiados;
}

char* posicion_a_string (t_posicion posicion) {
	char* posicionX = string_itoa(posicion.X);
	char* posicionY = string_itoa(posicion.Y);

	char* dupla = malloc (strlen(posicionX) + strlen(posicionY) + 2); // Dos más para el '-' y el '\0'

	strcat(strcat(strcpy(dupla, posicionX), "-"), posicionY);

	free(posicionX);
	free(posicionY);

	return dupla;
}

char* metadata_traer (char* ruta_metadata, char* pokemon, int32_t* tam_alojamiento) {
	struct stat estado_archivo;
	stat(ruta_metadata, &estado_archivo);

	*tam_alojamiento = estado_archivo.st_size;

	char* stream = malloc(estado_archivo.st_size + 1);

	sem_wait (dictionary_get(semaforos, pokemon));

	FILE* metadata = abrir_para (ruta_metadata, "r");

	fread(stream, estado_archivo.st_size, 1, metadata);

	fclose(metadata);

	stream[estado_archivo.st_size] = '\0';

	sem_post (dictionary_get(semaforos, pokemon));

	return stream;
}

char* copiar_stream_con_tamanio(FILE* archivo_lectura, char* ruta, int32_t* tamanio_copiado) {
	struct stat estado_archivo;
	stat(ruta, &estado_archivo);

	char* stream = malloc(estado_archivo.st_size);

	fread(stream, estado_archivo.st_size, 1, archivo_lectura);

	if (stream[estado_archivo.st_size - 1] == '#') {
		*tamanio_copiado = estado_archivo.st_size - 1;
		stream = realloc(stream, *tamanio_copiado);
	}
	else *tamanio_copiado = estado_archivo.st_size;

	return stream;
}

/****************************************************************************/
/***************************ENVIO DE MENSAJES********************************/
/****************************************************************************/

void enviar_appeared (char* pokemon, char* x, char* y, char* mensaje_id){
	int32_t operacion = 0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje = 0;
	sem_wait(&envio_respuesta);
	int32_t broker = crear_conexion(ip_broker,puerto_broker);
	sem_post(&envio_respuesta);
	if(broker == 0){
		log_error(logger_GC, "Error al enviar appeared al Broker");
	}else{
		sem_wait(&envio_respuesta);
		enviar_handshake(2, broker);
		sem_post(&envio_respuesta);
		if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
			if(operacion == ACK){
				recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
				recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				sem_wait(&envio_respuesta);
				enviar_appeared_pokemon(pokemon, x, y, mensaje_id, broker);
				sem_post(&envio_respuesta);
				if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){ //no lo uso pero el broker lo envia
					if(operacion == ACK){
						recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
					}
				}
			}
		}
	}
	liberar_conexion(broker);
}

void enviar_caught (char* id_mensaje_correlativo, char * fueAtrapado) {
	int32_t operacion = 0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje = 0;
	sem_wait(&envio_respuesta);
	int32_t broker = crear_conexion(ip_broker,puerto_broker);
	sem_post(&envio_respuesta);
	if(broker == 0){
		log_error(logger_GC, "Error al enviar caught al Broker");
	}else{
		sem_wait(&envio_respuesta);
		enviar_handshake(2, broker);
		sem_post(&envio_respuesta);
		if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
			if(operacion == ACK){
				recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
				recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				sem_wait(&envio_respuesta);
				enviar_caught_pokemon(id_mensaje_correlativo, fueAtrapado, broker);
				sem_post(&envio_respuesta);
				if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){ //no lo uso pero el broker lo envia
					if(operacion == ACK){
						recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
					}
				}
			}
		}
	}
	liberar_conexion(broker);
}

void enviar_localized (t_list* posiciones, t_pokemon pokemon, int32_t id_mensaje_correlativo) {
	int32_t operacion = 0;
	int32_t tamanio_estructura = 0;
	int32_t id_mensaje = 0;
	sem_wait(&envio_respuesta);
	int32_t broker = crear_conexion(ip_broker,puerto_broker);
	sem_post(&envio_respuesta);
	if(broker == 0){
		log_error(logger_GC, "Error al enviar caught al Broker");
	}else{
		sem_wait(&envio_respuesta);
		enviar_handshake(2, broker);
		sem_post(&envio_respuesta);
		if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) != -1){
			if(operacion == ACK){
				recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
				recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
				sem_wait(&envio_respuesta);
				enviar_localized_pokemon (&pokemon, posiciones, id_mensaje_correlativo, broker);
				sem_post(&envio_respuesta);
				if(recv(broker, &operacion, sizeof(int32_t), MSG_WAITALL) > 0){ //no lo uso pero el broker lo envia
					if(operacion == ACK){
						recv(broker, &tamanio_estructura, sizeof(int32_t), MSG_WAITALL);
						recv(broker, &id_mensaje, sizeof(int32_t), MSG_WAITALL);
					}
				}
			}
		}
	}
	liberar_conexion(broker);
}


















