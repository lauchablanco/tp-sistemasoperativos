################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.c 

OBJS += \
./UniversoPokemon/universo.o 

C_DEPS += \
./UniversoPokemon/universo.d 


# Each subdirectory must supply rules for building sources it contributes
UniversoPokemon/universo.o: /home/utnso/workspace/tp-2020-1c-5rona/OurLibraries/UniversoPokemon/universo.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


