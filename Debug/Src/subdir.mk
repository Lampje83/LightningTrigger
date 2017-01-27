################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/adc.c \
../Src/dma.c \
../Src/gpio.c \
../Src/main.c \
../Src/rtc.c \
../Src/spi.c \
../Src/stm32f1xx_hal_msp.c \
../Src/stm32f1xx_it.c \
../Src/ui.c \
../Src/usb_device.c \
../Src/usbd_cdc_if.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c 

OBJS += \
./Src/adc.o \
./Src/dma.o \
./Src/gpio.o \
./Src/main.o \
./Src/rtc.o \
./Src/spi.o \
./Src/stm32f1xx_hal_msp.o \
./Src/stm32f1xx_it.o \
./Src/ui.o \
./Src/usb_device.o \
./Src/usbd_cdc_if.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o 

C_DEPS += \
./Src/adc.d \
./Src/dma.d \
./Src/gpio.d \
./Src/main.d \
./Src/rtc.d \
./Src/spi.d \
./Src/stm32f1xx_hal_msp.d \
./Src/stm32f1xx_it.d \
./Src/ui.d \
./Src/usb_device.d \
./Src/usbd_cdc_if.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))" -DUSE_HAL_DRIVER -DSTM32F103xB -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Inc" -I../Drivers/SH1106 -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/STM32F1xx_HAL_Driver/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/CMSIS/Include" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/Beheerder/Projects/STM32/LightningTrigger/Inc"  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


