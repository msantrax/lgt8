################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/snmp/asn1_dec.c \
../lwip/src/core/snmp/asn1_enc.c \
../lwip/src/core/snmp/mib2.c \
../lwip/src/core/snmp/mib_structs.c \
../lwip/src/core/snmp/msg_in.c \
../lwip/src/core/snmp/msg_out.c 

OBJS += \
./lwip/src/core/snmp/asn1_dec.o \
./lwip/src/core/snmp/asn1_enc.o \
./lwip/src/core/snmp/mib2.o \
./lwip/src/core/snmp/mib_structs.o \
./lwip/src/core/snmp/msg_in.o \
./lwip/src/core/snmp/msg_out.o 

C_DEPS += \
./lwip/src/core/snmp/asn1_dec.d \
./lwip/src/core/snmp/asn1_enc.d \
./lwip/src/core/snmp/mib2.d \
./lwip/src/core/snmp/mib_structs.d \
./lwip/src/core/snmp/msg_in.d \
./lwip/src/core/snmp/msg_out.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/core/snmp/%.o: ../lwip/src/core/snmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK64FN1M0VDC12" -DDEBUG -DFRDM -DFREEDOM -I../CMSIS -I"/Bascon/NXP/kds_workspace/lgt8/source" -I"/Bascon/NXP/kds_workspace/lgt8/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port/arch" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/netif" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/posix" -I../board -I../drivers -I../startup -I../utilities -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


