#include "mensajes.h"

void enviar_new_pokemon(char* pokemon, char* x, char* y, char* cantidad, char* id_mensaje, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_posicion  posicion;
	posicion.X = (int32_t) atoi (x);
	posicion.Y = (int32_t) atoi (y);

	t_pokemon p_pokemon;
	p_pokemon.size_Nombre = string_length(pokemon) +1;
	p_pokemon.nombre = pokemon;

	t_New new;
	new.cant = (int32_t) atoi (cantidad);
	new.posicion = posicion;
	new.pokemon = p_pokemon;

	paquete->buffer->size = tamanio_new (&new);
	paquete->buffer->id_Mensaje = (int32_t) atoi (id_mensaje);
	paquete->buffer->stream = &new;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_new (paquete, &bytes_a_enviar, &new);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

void enviar_appeared_pokemon (char* pokemon, char* x, char* y, char* id_mensaje_correlativo, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = APPEARED_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_posicion  posicion;
	posicion.X = (int32_t) atoi (x);
	posicion.Y = (int32_t) atoi (y);

	t_pokemon p_pokemon;
	p_pokemon.size_Nombre = string_length(pokemon) +1;
	p_pokemon.nombre = pokemon;

	t_Appeared app;
	app.posicion = posicion;
	app.pokemon = p_pokemon;

	paquete->buffer->size = tamanio_appeared (&app);
	paquete->buffer->id_Mensaje = (int32_t) atoi (id_mensaje_correlativo);
	paquete->buffer->stream = &app;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_appeared (paquete, &bytes_a_enviar, &app);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);

	return;
}


void enviar_catch_pokemon(char* pokemon, char* x, char* y, char* id_mensaje, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = CATCH_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_pokemon p_pokemon;
	p_pokemon.size_Nombre = strlen(pokemon) +1;
	p_pokemon.nombre = pokemon;

	t_posicion posicion;
	posicion.X = (int32_t) atoi (x);
	posicion.Y = (int32_t) atoi (y);

	t_Catch catch;
	catch.posicion = posicion;
	catch.pokemon = p_pokemon;

	paquete->buffer->size = tamanio_catch(&catch);
	paquete->buffer->id_Mensaje = (int32_t) atoi (id_mensaje);
	paquete->buffer->stream = &catch;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_catch (paquete, &bytes_a_enviar, &catch);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);

	return;
}

void enviar_caught_pokemon(char* id_mensaje_correlativo, char * fueAtrapado, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = CAUGHT_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_Caught caught;
	if (strcmp(fueAtrapado, "OK") == 0 || strcmp(fueAtrapado, "1") == 0) caught.fueAtrapado = 1;
	else if (strcmp(fueAtrapado, "FAIL") == 0 || strcmp(fueAtrapado, "0") == 0) caught.fueAtrapado = 0;

	paquete->buffer->size = tamanio_caught(&caught);
	paquete->buffer->id_Mensaje = (int32_t) atoi (id_mensaje_correlativo);
	paquete->buffer->stream = &caught;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_caught(paquete, &bytes_a_enviar, &caught);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

void enviar_get_pokemon(char* pokemon, char* id_mensaje, int32_t socket_cliente)
{
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_pokemon p_pokemon;
	p_pokemon.size_Nombre = strlen(pokemon) +1;
	p_pokemon.nombre = pokemon;

	t_Get get;
	get.pokemon = p_pokemon;

	paquete->buffer->size = tamanio_get(&get);
	paquete->buffer->id_Mensaje = (int32_t) atoi (id_mensaje);
	paquete->buffer->stream = &get;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_get (paquete, &bytes_a_enviar, &get);

	send(socket_cliente, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);

}

void enviar_localized_pokemon (t_pokemon* pokemon, t_list* lista_de_posiciones, int32_t id_mensaje, int32_t socket_hacia) {
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = LOCALIZED_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	t_Localized localized;
	localized.pokemon = *pokemon;
	localized.listaPosiciones = lista_de_posiciones;

	paquete->buffer->size = tamanio_localized(&localized);
	paquete->buffer->id_Mensaje = id_mensaje;
	paquete->buffer->stream = &localized;

	int32_t bytes_a_enviar;
	void *paqueteSerializado = serializar_paquete_localized (paquete, &bytes_a_enviar, &localized);

	send(socket_hacia, paqueteSerializado, bytes_a_enviar, 0);

	free(paqueteSerializado);
	free(paquete->buffer);
	free(paquete);
}

