
# This is a GNU Makefile.

# It can be used to compile an OpenCL program with
# the Altera Beta OpenCL Development Kit.
# See README.txt for more information.


# You must configure ALTERAOCLSDKROOT to point the root directory of the Altera SDK for OpenCL
# software installation.
# See doc/getting_started.txt for more information on installing and
# configuring the Altera SDK for OpenCL.


# Creating a static library
TARGET = convolution_image

# Where is the Altera SDK for OpenCL software?
ifeq ($(wildcard $(ALTERAOCLSDKROOT)),)
$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation)
endif
ifeq ($(wildcard $(ALTERAOCLSDKROOT)/host/include/CL/opencl.h),)
$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation.)
endif

# Libraries to use, objects to compile
SRCS = convolution_main.cpp \
		gFreeImage.cpp
SRCS_FILES = $(foreach F, $(SRCS), host/src/$(F))  

COMMON_DIR=../common
COMMONFILE_LIB=$(COMMON_DIR)/lib
COMMONFILE_INC=$(COMMON_DIR)/inc
COMMONFILE_SRC=$(COMMON_DIR)/src
COMMON_FILES = $(COMMONFILE_SRC)/tool.cpp
FREEIMAGE_INC = -I$(COMMONFILE_INC)
FREEIMAGE_LIB= -L$(COMMONFILE_LIB) -lfreeimage
#			../common/src/AOCL_Utils.cpp
# arm cross compiler
CROSS-COMPILE = arm-linux-gnueabihf-

# OpenCL compile and link flags.
AOCL_COMPILE_CONFIG=$(shell aocl compile-config --arm) -I$(COMMONFILE_INC) -Ihost/inc
AOCL_LINK_CONFIG=$(shell aocl link-config --arm) -lrt 


# Make it all!
all : 
	$(CROSS-COMPILE)g++ $(SRCS_FILES) $(COMMON_FILES) -o $(TARGET)  $(AOCL_COMPILE_CONFIG) $(AOCL_LINK_CONFIG) $(FREEIMAGE_INC) $(FREEIMAGE_LIB)


# Standard make targets
clean :
	@rm -f *.o $(TARGET)
