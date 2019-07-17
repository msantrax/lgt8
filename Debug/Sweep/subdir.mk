################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Sweep/sweep.cpp \
../Sweep/sweep_sm.cpp 

OBJS += \
./Sweep/sweep.o \
./Sweep/sweep_sm.o 

CPP_DEPS += \
./Sweep/sweep.d \
./Sweep/sweep_sm.d 


# Each subdirectory must supply rules for building sources it contributes
Sweep/%.o: ../Sweep/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -D"CPU_MK64FN1M0VDC12" -I../CMSIS -I"/Bascon/NXP/kds_workspace/lgt8/Sweep" -I"/Bascon/NXP/kds_workspace/lgt8/Servo" -I"/Bascon/NXP/kds_workspace/lgt8/Monitor" -I"/Bascon/NXP/kds_workspace/lgt8/source" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/src/include" -I"/Bascon/NXP/kds_workspace/lgt8/lwip/port" -I"/Bascon/NXP/kds_workspace/lgt8/lwip" -I../board -I../drivers -I../startup -I../utilities -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


