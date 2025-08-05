################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/ADC2.cpp \
../Core/Src/axis.cpp \
../Core/Src/commando_interpreter.cpp \
../Core/Src/controller_error.cpp \
../Core/Src/current_control.cpp \
../Core/Src/data_recorder.cpp \
../Core/Src/main.cpp \
../Core/Src/monitor.cpp \
../Core/Src/parameter.cpp \
../Core/Src/pid.cpp \
../Core/Src/pos_sensor.cpp \
../Core/Src/position_control.cpp \
../Core/Src/set_pwm.cpp \
../Core/Src/trajectory.cpp 

C_SRCS += \
../Core/Src/adc.c \
../Core/Src/app_freertos.c \
../Core/Src/dma.c \
../Core/Src/fdcan.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/main.c \
../Core/Src/spi.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/app_freertos.d \
./Core/Src/dma.d \
./Core/Src/fdcan.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/main.d \
./Core/Src/spi.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d 

OBJS += \
./Core/Src/ADC2.o \
./Core/Src/adc.o \
./Core/Src/app_freertos.o \
./Core/Src/axis.o \
./Core/Src/commando_interpreter.o \
./Core/Src/controller_error.o \
./Core/Src/current_control.o \
./Core/Src/data_recorder.o \
./Core/Src/dma.o \
./Core/Src/fdcan.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/main.o \
./Core/Src/monitor.o \
./Core/Src/parameter.o \
./Core/Src/pid.o \
./Core/Src/pos_sensor.o \
./Core/Src/position_control.o \
./Core/Src/set_pwm.o \
./Core/Src/spi.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/tim.o \
./Core/Src/trajectory.o \
./Core/Src/usart.o 

CPP_DEPS += \
./Core/Src/ADC2.d \
./Core/Src/axis.d \
./Core/Src/commando_interpreter.d \
./Core/Src/controller_error.d \
./Core/Src/current_control.d \
./Core/Src/data_recorder.d \
./Core/Src/main.d \
./Core/Src/monitor.d \
./Core/Src/parameter.d \
./Core/Src/pid.d \
./Core/Src/pos_sensor.d \
./Core/Src/position_control.d \
./Core/Src/set_pwm.d \
./Core/Src/trajectory.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.cpp Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../CANopenNode/ -I../CANopenNode_STM32/ -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../CANopenNode/ -I../CANopenNode_STM32/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ADC2.cyclo ./Core/Src/ADC2.d ./Core/Src/ADC2.o ./Core/Src/ADC2.su ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/app_freertos.cyclo ./Core/Src/app_freertos.d ./Core/Src/app_freertos.o ./Core/Src/app_freertos.su ./Core/Src/axis.cyclo ./Core/Src/axis.d ./Core/Src/axis.o ./Core/Src/axis.su ./Core/Src/commando_interpreter.cyclo ./Core/Src/commando_interpreter.d ./Core/Src/commando_interpreter.o ./Core/Src/commando_interpreter.su ./Core/Src/controller_error.cyclo ./Core/Src/controller_error.d ./Core/Src/controller_error.o ./Core/Src/controller_error.su ./Core/Src/current_control.cyclo ./Core/Src/current_control.d ./Core/Src/current_control.o ./Core/Src/current_control.su ./Core/Src/data_recorder.cyclo ./Core/Src/data_recorder.d ./Core/Src/data_recorder.o ./Core/Src/data_recorder.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/fdcan.cyclo ./Core/Src/fdcan.d ./Core/Src/fdcan.o ./Core/Src/fdcan.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/monitor.cyclo ./Core/Src/monitor.d ./Core/Src/monitor.o ./Core/Src/monitor.su ./Core/Src/parameter.cyclo ./Core/Src/parameter.d ./Core/Src/parameter.o ./Core/Src/parameter.su ./Core/Src/pid.cyclo ./Core/Src/pid.d ./Core/Src/pid.o ./Core/Src/pid.su ./Core/Src/pos_sensor.cyclo ./Core/Src/pos_sensor.d ./Core/Src/pos_sensor.o ./Core/Src/pos_sensor.su ./Core/Src/position_control.cyclo ./Core/Src/position_control.d ./Core/Src/position_control.o ./Core/Src/position_control.su ./Core/Src/set_pwm.cyclo ./Core/Src/set_pwm.d ./Core/Src/set_pwm.o ./Core/Src/set_pwm.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32g4xx_hal_msp.cyclo ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.cyclo ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g4xx.cyclo ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/trajectory.cyclo ./Core/Src/trajectory.d ./Core/Src/trajectory.o ./Core/Src/trajectory.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

