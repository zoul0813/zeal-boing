BIN=boing.bin


# include $(ZVB_SDK_PATH)/sdcc/base_sdcc.mk
include $(ZGDK_PATH)/base_sdcc.mk

all::
	ls -l $(OUTPUT_DIR)/$(BIN)