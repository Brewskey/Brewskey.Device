# This file is a makefile included from the top level makefile which
# defines the sources built for the target.

# Define the prefix to this directory. 
# Note: The name must be unique within this build and should be
#       based on the root of the project
TARGET_NDEF_PATH = libraries/NDEF

# Add include paths.
INCLUDE_DIRS += $(TARGET_NDEF_PATH)/inc

# C source files included in this build.
CSRC +=

# C++ source files included in this build.
CPPSRC += $(call target_files,$(TARGET_NDEF_PATH)/src,*.cpp)


# ASM source files included in this build.
ASRC +=