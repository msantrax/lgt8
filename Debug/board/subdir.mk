################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c 

OBJS += \
./board/board.o \
./board/clock_config.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK64FN1M0VDC12" -DDEBUG -DFRDM -DFREEDOM -I../CMSIS -I"/Bascon/NXP/kds_workspace/lgt8/source" -I"/Bascon/NXP/kds_workspace/lgt8/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port/arch" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv4/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/ipv6/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/lwip" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/netif" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include/posix" -I../board -I../drivers -I../startup -I../utilities -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


