#ifndef SRC_CONEXION_H_
#define SRC_CONEXION_H_


#include<commons/config.h>
#include<stdint.h>
#include<commons/log.h>
#include "/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/sockets.h"

int32_t conexionBroker();
int32_t conexionTeam();
int32_t conexionGameCard();

#endif
