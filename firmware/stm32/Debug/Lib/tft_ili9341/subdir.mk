################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/tft_ili9341/ILI9341_GFX.c \
../Lib/tft_ili9341/ILI9341_STM32_Driver.c \
../Lib/tft_ili9341/fonts.c 

OBJS += \
./Lib/tft_ili9341/ILI9341_GFX.o \
./Lib/tft_ili9341/ILI9341_STM32_Driver.o \
./Lib/tft_ili9341/fonts.o 

C_DEPS += \
./Lib/tft_ili9341/ILI9341_GFX.d \
./Lib/tft_ili9341/ILI9341_STM32_Driver.d \
./Lib/tft_ili9341/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/tft_ili9341/%.o Lib/tft_ili9341/%.su Lib/tft_ili9341/%.cyclo: ../Lib/tft_ili9341/%.c Lib/tft_ili9341/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/delay" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/dht_11" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/lm_393" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/pir_am312" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/tft_ili9341" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Lib/uart_test" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/Application" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/include" -I"D:/anh_khoa/stm32_workspace/pcb_board/BTL_LTN/FreeRTOS/portable/GCC/ARM_CM3" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Lib-2f-tft_ili9341

clean-Lib-2f-tft_ili9341:
	-$(RM) ./Lib/tft_ili9341/ILI9341_GFX.cyclo ./Lib/tft_ili9341/ILI9341_GFX.d ./Lib/tft_ili9341/ILI9341_GFX.o ./Lib/tft_ili9341/ILI9341_GFX.su ./Lib/tft_ili9341/ILI9341_STM32_Driver.cyclo ./Lib/tft_ili9341/ILI9341_STM32_Driver.d ./Lib/tft_ili9341/ILI9341_STM32_Driver.o ./Lib/tft_ili9341/ILI9341_STM32_Driver.su ./Lib/tft_ili9341/fonts.cyclo ./Lib/tft_ili9341/fonts.d ./Lib/tft_ili9341/fonts.o ./Lib/tft_ili9341/fonts.su

.PHONY: clean-Lib-2f-tft_ili9341

