################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32h743zitx.s 

S_DEPS += \
./Core/Startup/startup_stm32h743zitx.d 

OBJS += \
./Core/Startup/startup_stm32h743zitx.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/startup_stm32h743zitx.o: ../Core/Startup/startup_stm32h743zitx.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -c -I"/home/light/STM32CubeIDE/workspace_1.6.1/test/Core" -I"/home/light/STM32CubeIDE/workspace_1.6.1/test/Core/Cef" -x assembler-with-cpp -MMD -MP -MF"Core/Startup/startup_stm32h743zitx.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

