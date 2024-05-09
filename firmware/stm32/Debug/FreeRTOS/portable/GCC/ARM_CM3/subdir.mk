################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/portable/GCC/ARM_CM3/port.c 

OBJS += \
./FreeRTOS/portable/GCC/ARM_CM3/port.o 

C_DEPS += \
./FreeRTOS/portable/GCC/ARM_CM3/port.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/GCC/ARM_CM3/%.o FreeRTOS/portable/GCC/ARM_CM3/%.su FreeRTOS/portable/GCC/ARM_CM3/%.cyclo: ../FreeRTOS/portable/GCC/ARM_CM3/%.c FreeRTOS/portable/GCC/ARM_CM3/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/delay" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/dht_11" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/lm_393" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/pir_am312" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/tft_ili9341" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/uart_test" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Application" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/include" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/portable/GCC/ARM_CM3" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM3

clean-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM3:
	-$(RM) ./FreeRTOS/portable/GCC/ARM_CM3/port.cyclo ./FreeRTOS/portable/GCC/ARM_CM3/port.d ./FreeRTOS/portable/GCC/ARM_CM3/port.o ./FreeRTOS/portable/GCC/ARM_CM3/port.su

.PHONY: clean-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM3

