################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/auth.c \
../lwip/src/netif/ppp/chap.c \
../lwip/src/netif/ppp/chpms.c \
../lwip/src/netif/ppp/fsm.c \
../lwip/src/netif/ppp/ipcp.c \
../lwip/src/netif/ppp/lcp.c \
../lwip/src/netif/ppp/magic.c \
../lwip/src/netif/ppp/md5.c \
../lwip/src/netif/ppp/pap.c \
../lwip/src/netif/ppp/ppp.c \
../lwip/src/netif/ppp/ppp_md5.c \
../lwip/src/netif/ppp/ppp_oe.c \
../lwip/src/netif/ppp/randm.c \
../lwip/src/netif/ppp/vj.c 

OBJS += \
./lwip/src/netif/ppp/auth.o \
./lwip/src/netif/ppp/chap.o \
./lwip/src/netif/ppp/chpms.o \
./lwip/src/netif/ppp/fsm.o \
./lwip/src/netif/ppp/ipcp.o \
./lwip/src/netif/ppp/lcp.o \
./lwip/src/netif/ppp/magic.o \
./lwip/src/netif/ppp/md5.o \
./lwip/src/netif/ppp/pap.o \
./lwip/src/netif/ppp/ppp.o \
./lwip/src/netif/ppp/ppp_md5.o \
./lwip/src/netif/ppp/ppp_oe.o \
./lwip/src/netif/ppp/randm.o \
./lwip/src/netif/ppp/vj.o 

C_DEPS += \
./lwip/src/netif/ppp/auth.d \
./lwip/src/netif/ppp/chap.d \
./lwip/src/netif/ppp/chpms.d \
./lwip/src/netif/ppp/fsm.d \
./lwip/src/netif/ppp/ipcp.d \
./lwip/src/netif/ppp/lcp.d \
./lwip/src/netif/ppp/magic.d \
./lwip/src/netif/ppp/md5.d \
./lwip/src/netif/ppp/pap.d \
./lwip/src/netif/ppp/ppp.d \
./lwip/src/netif/ppp/ppp_md5.d \
./lwip/src/netif/ppp/ppp_oe.d \
./lwip/src/netif/ppp/randm.d \
./lwip/src/netif/ppp/vj.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/netif/ppp/%.o: ../lwip/src/netif/ppp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK64FN1M0VDC12" -DDEBUG -DFRDM -DFREEDOM -I../CMSIS -I"/Bascon/NXP/kds_workspace/lgt8/source" -I"/Bascon/NXP/kds_workspace/lgt8/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port/arch" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/netif" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/posix" -I../board -I../drivers -I../startup -I../utilities -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


