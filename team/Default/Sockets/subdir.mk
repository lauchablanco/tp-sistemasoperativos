################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/mensajes.c \
/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/sockets.c 

OBJS += \
./Sockets/mensajes.o \
./Sockets/sockets.o 

C_DEPS += \
./Sockets/mensajes.d \
./Sockets/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
Sockets/mensajes.o: /home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/mensajes.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Sockets/sockets.o: /home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/Sockets/sockets.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


