cmake_minimum_required(VERSION 3.15)

project(bbs-fw C)

set(TARGET_CONTROLLER "BBSHD" CACHE STRING "Target controller: BBSHD,BBS02,TSDZ2,TSDZ8")

if (${TARGET_CONTROLLER} STREQUAL "BBSHD")
    add_link_options(--xram-size 3840)
endif()

if (${TARGET_CONTROLLER} STREQUAL "BBS02")
    add_link_options(--xram-size 1792)
endif()

if (${TARGET_CONTROLLER} STREQUAL "TSDZ2")
    add_subdirectory(lib/stm8s)
endif()

set(SOURCE_FILES
    "src/adc.h"
    "src/app.c"
    "src/app.h"
    "src/battery.c"
    "src/battery.h"
    "src/cfgstore.c"
    "src/cfgstore.h"
    "src/eeprom.h"
    "src/eventlog.c"
    "src/eventlog.h"
    "src/extcom.c"
    "src/extcom.h"
    "src/fwconfig.h"
    "src/interrupt.h"
    "src/lights.h"
    "src/main.c"
    "src/motor.h"
    "src/sensors.h"
    "src/system.h"
    "src/throttle.c"
    "src/throttle.h"
    "src/timers.h"
    "src/uart.h"
    "src/util.h"
    "src/version.h"
    "src/watchdog.h"
)

if (${TARGET_CONTROLLER} STREQUAL "BBSHD" OR ${TARGET_CONTROLLER} STREQUAL "BBS02")
    list(APPEND SOURCE_FILES
        "src/bbsx/adc.c"
        "src/bbsx/cpu.h"
        "src/bbsx/eeprom.c"
        "src/bbsx/interrupt.h"
        "src/bbsx/lights.c"
        "src/bbsx/motor.c"
        "src/bbsx/pins.h"
        "src/bbsx/sensors.c"
        "src/bbsx/stc15.h"
        "src/bbsx/system.c"
        "src/bbsx/timers.c"
        "src/bbsx/timers.h"
        "src/bbsx/uart_motor.h"
        "src/bbsx/uart.c"
        "src/bbsx/watchdog.c"
    )
endif()

if (${TARGET_CONTROLLER} STREQUAL "TSDZ2")
    list(APPEND SOURCE_FILES
        "src/tsdz2/adc.c"
        "src/tsdz2/cpu.h"
        "src/tsdz2/eeprom.c"
        "src/tsdz2/interrupt.h"
        "src/tsdz2/lights.c"
        "src/tsdz2/motor.c"
        "src/tsdz2/pins.h"
        "src/tsdz2/sensors.c"
        "src/tsdz2/stm8.h"
        "src/tsdz2/system.c"
        "src/tsdz2/timers.c"
        "src/tsdz2/timers.h"
        "src/tsdz2/torquesensor.c"
        "src/tsdz2/uart.c"
        "src/tsdz2/watchdog.c"
    )
endif()

add_executable(bbs-fw ${SOURCE_FILES})

target_compile_definitions(bbs-fw PRIVATE ${TARGET_CONTROLLER})
target_include_directories(bbs-fw PRIVATE src)

if (${TARGET_CONTROLLER} STREQUAL "TSDZ2")
    target_link_libraries(bbs-fw stm8s)
endif()


# Generate hex file (SDCC)
if (${CMAKE_C_COMPILER} STREQUAL "sdcc" OR ${TARGET_CONTROLLER} STREQUAL "BBS02" OR ${TARGET_CONTROLLER} STREQUAL "TSDZ2")
    add_custom_command(
        POST_BUILD
        COMMENT Generating bbs-fw.hex
        TARGET bbs-fw
        COMMAND packihx bbs-fw.ihx > bbs-fw.hex
    )
endif()
