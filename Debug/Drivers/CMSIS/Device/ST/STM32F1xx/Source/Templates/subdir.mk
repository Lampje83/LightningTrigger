################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c 

OBJS += \
./Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.o 

C_DEPS += \
./Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/%.o: ../Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))" -DUSE_HAL_DRIVER -DSTM32F103xB -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Inc" -I../Drivers/SH1106 -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/STM32F1xx_HAL_Driver/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/CMSIS/Include" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Inc"  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


