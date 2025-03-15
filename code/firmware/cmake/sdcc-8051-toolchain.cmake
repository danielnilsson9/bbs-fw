# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR 8051)

set(CMAKE_C_FLAGS_INIT "-mmcs51 --model-large --std-sdcc11 -Ddouble=float")
set(CMAKE_EXE_LINKER_FLAGS_INIT "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# which compilers to use for C and ASM
set(CMAKE_C_COMPILER sdcc)

find_program (SDCC NAMES sdcc)
get_filename_component(SDCC_BIN_DIR ${SDCC} DIRECTORY)
get_filename_component(SDCC_PATH_DIR ${SDCC_BIN_DIR} DIRECTORY)

# here is the target environment is located
set(CMAKE_FIND_ROOT_PATH  ${SDCC_PATH_DIR}/usr/share/sdcc)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_ASM_OUTPUT_EXTENSION ".rel")
