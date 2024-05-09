################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/dht_11/dht_11.c 

OBJS += \
./Lib/dht_11/dht_11.o 

C_DEPS += \
./Lib/dht_11/dht_11.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/dht_11/%.o Lib/dht_11/%.su Lib/dht_11/%.cyclo: ../Lib/dht_11/%.c Lib/dht_11/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/delay" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/dht_11" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/lm_393" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/pir_am312" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/tft_ili9341" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/uart_test" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Application" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/include" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/portable/GCC/ARM_CM3" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Lib-2f-dht_11

clean-Lib-2f-dht_11:
	-$(RM) ./Lib/dht_11/dht_11.cyclo ./Lib/dht_11/dht_11.d ./Lib/dht_11/dht_11.o ./Lib/dht_11/dht_11.su

.PHONY: clean-Lib-2f-dht_11

