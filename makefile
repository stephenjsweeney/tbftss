CC = gcc
EXEEXT = 
BIN_DIR = /usr/bin
DATA_DIR = /opt/tbftss

SEARCHPATH += src/plat/unix
OBJS += unixInit.o

include common.mk
