################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/ipv4/autoip.c \
../lwip/src/core/ipv4/icmp.c \
../lwip/src/core/ipv4/igmp.c \
../lwip/src/core/ipv4/inet.c \
../lwip/src/core/ipv4/inet_chksum.c \
../lwip/src/core/ipv4/ip.c \
../lwip/src/core/ipv4/ip_addr.c \
../lwip/src/core/ipv4/ip_frag.c 

OBJS += \
./lwip/src/core/ipv4/autoip.o \
./lwip/src/core/ipv4/icmp.o \
./lwip/src/core/ipv4/igmp.o \
./lwip/src/core/ipv4/inet.o \
./lwip/src/core/ipv4/inet_chksum.o \
./lwip/src/core/ipv4/ip.o \
./lwip/src/core/ipv4/ip_addr.o \
./lwip/src/core/ipv4/ip_frag.o 

C_DEPS += \
./lwip/src/core/ipv4/autoip.d \
./lwip/src/core/ipv4/icmp.d \
./lwip/src/core/ipv4/igmp.d \
./lwip/src/core/ipv4/inet.d \
./lwip/src/core/ipv4/inet_chksum.d \
./lwip/src/core/ipv4/ip.d \
./lwip/src/core/ipv4/ip_addr.d \
./lwip/src/core/ipv4/ip_frag.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/core/ipv4/%.o: ../lwip/src/core/ipv4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK64FN1M0VDC12" -DDEBUG -DFRDM -DFREEDOM -I../CMSIS -I"/Bascon/NXP/kds_workspace/lgt8/source" -I"/Bascon/NXP/kds_workspace/lgt8/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port/arch" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/netif" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/posix" -I../board -I../drivers -I../startup -I../utilities -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


