################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Cef/Command.cpp 

OBJS += \
./Core/Cef/Command.o 

CPP_DEPS += \
./Core/Cef/Command.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Cef/Command.o: ../Core/Cef/Command.cpp Core/Cef/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/light/STM32CubeIDE/workspace_1.6.1/test/Core" -I"/home/light/STM32CubeIDE/workspace_1.6.1/test/Core/Cef" -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Core/Cef/Command.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

