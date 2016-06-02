################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SW-TM4C-2.1.2.111/utils/uartstdio.c 

OBJS += \
./SW-TM4C-2.1.2.111/utils/uartstdio.o 

C_DEPS += \
./SW-TM4C-2.1.2.111/utils/uartstdio.d 


# Each subdirectory must supply rules for building sources it contributes
SW-TM4C-2.1.2.111/utils/%.o: ../SW-TM4C-2.1.2.111/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DPART_TM4C123GH6PM -I"/home/user/workspace/7SEG2/SW-TM4C-2.1.2.111" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


