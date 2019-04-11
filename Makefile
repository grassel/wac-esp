#
# Per ESP-IF convention, the  Makefile must reside in the priject base directory
# but all c/h source code files are in 'component' directories, of whihc
# 'main' component has a special meaning.
#

PROJECT_NAME := wac

#COMPONENT_DIRS := main

ifneq (,$(ESP_PLATFORM))
  CFLAGS += -DESP_TARGET
  include $(IDF_PATH)/make/project.mk
else
  ESP_TARGET=
endif

# libc or fooboot
PLATFORM = libc

#CFLAGS ?= -O2 -Wall -Werror -Wextra -MMD -MP
CFLAGS += -O2 -Wall -Werror -MMD -MP -DPLATFORM=1

# enable this define to compile with low memory profile that fits to ESP32 dev board.
# this config can also be used for testing purposes on Linux
CFLAGS += -DLOW_MEMORY_CONFIG


ifeq (,$(ESP_PLATFORM))
  CC = gcc $(CFLAGS) -std=gnu99 -m32 -g
  OBJ = out/wac.o out/wa.o out/util.o out/platform.o out/thunk.o out/wifi_http_comm.o
endif

#  dependencies
out/util.o: main/util.h
out/wa.o: main/wa.h main/util.h main/platform.h 
out/thunk.o: main/wa.h main/thunk.h
out/platform.o: main/platform.h main/util.h
out/app_wifi_http.o: out/app_wifi_http.h  
out/app_wac.o: main/app_wac.h main/util.h main/platform.h main/wa.h  
out/app_main.o: main/app_wac.h main/app_wifi_http.h 

ifeq (,$(ESP_PLATFORM))

all: out/wac

out/wac: $(OBJ)  

out/%.o: main/%.c
	$(CC) -c $(filter %.c,$^) -o $@

out/wac : $(OBJ)
	$(CC) -rdynamic -Wl,--no-as-needed -o $@ -Wl,--start-group $^ -Wl,--end-group -lm 

clean ::
	rm -f out/* examples_wast/*.wasm

else

endif




