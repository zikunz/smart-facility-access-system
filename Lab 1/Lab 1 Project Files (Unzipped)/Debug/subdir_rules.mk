################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
gpio_if.obj: C:/ti/CC3200SDK_1.4.0/cc3200-sdk/example/common/gpio_if.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/Users/Zhu Zikun/Desktop/EEC 172 Embeded Systems/Lab Workspace/New folder/lab1" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/driverlib/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/inc/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/example/common/" --include_path="C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="gpio_if.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/Users/Zhu Zikun/Desktop/EEC 172 Embeded Systems/Lab Workspace/New folder/lab1" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/driverlib/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/inc/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/example/common/" --include_path="C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_ccs.obj: C:/ti/CC3200SDK_1.4.0/cc3200-sdk/example/common/startup_ccs.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/Users/Zhu Zikun/Desktop/EEC 172 Embeded Systems/Lab Workspace/New folder/lab1" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/driverlib/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/inc/" --include_path="C:/ti/CC3200SDK_1.4.0/cc3200-sdk/example/common/" --include_path="C:/ti/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="startup_ccs.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


