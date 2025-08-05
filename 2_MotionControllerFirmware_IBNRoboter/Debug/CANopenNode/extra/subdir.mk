################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CANopenNode/extra/CO_trace.c 

C_DEPS += \
./CANopenNode/extra/CO_trace.d 

OBJS += \
./CANopenNode/extra/CO_trace.o 


# Each subdirectory must supply rules for building sources it contributes
CANopenNode/extra/%.o CANopenNode/extra/%.su CANopenNode/extra/%.cyclo: ../CANopenNode/extra/%.c CANopenNode/extra/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../CANopenNode/ -I../CANopenNode_STM32/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CANopenNode-2f-extra

clean-CANopenNode-2f-extra:
	-$(RM) ./CANopenNode/extra/CO_trace.cyclo ./CANopenNode/extra/CO_trace.d ./CANopenNode/extra/CO_trace.o ./CANopenNode/extra/CO_trace.su

.PHONY: clean-CANopenNode-2f-extra

