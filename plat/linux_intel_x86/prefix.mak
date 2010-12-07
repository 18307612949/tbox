# architecture makefile configure

# prefix & suffix
BIN_PREFIX 		= 
BIN_SUFFIX 		= 

OBJ_PREFIX 		= 
OBJ_SUFFIX 		= .o

LIB_PREFIX 		= lib
LIB_SUFFIX 		= .a

DLL_PREFIX 		= lib
DLL_SUFFIX 		= .so

ASM_SUFFIX 		= .S

# tool
PRE 			= 
CC 				= icc
LD 				= icc
AR 				= xiar
STRIP 			=
RANLIB 			=
AS				= 
RM 				= rm -f
RMDIR 			= rm -rf
CP 				= cp
CPDIR 			= cp -r
MKDIR 			= mkdir -p
MAKE 			= make
PWD 			= pwd

# cflags
CFLAGS_RELEASE 	= -O2 -fomit-frame-pointer
CFLAGS_DEBUG 	= -g
CFLAGS 			= -c -Wall -fasm-blocks
CFLAGS-I 		= -I
CFLAGS-o 		= -o

# ldflags
LDFLAGS 		= -static
LDFLAGS-L 		= -L
LDFLAGS-l 		= -l
LDFLAGS-o 		= -o

# asflags
ASFLAGS 		= -c
ASFLAGS-I 		= -I
ASFLAGS-o 		= -o

# arflags
ARFLAGS 		= -cr

# share ldflags
SHFLAGS 		= -shared -Wl,-soname

# include sub-config
include 		$(PLAT_DIR)$(_)config.mak


