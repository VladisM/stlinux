cmake_minimum_required(VERSION 3.13)

project(board_test C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(TARGET_NAME tester)

set(CPU cortex-m4)
set(MCU "STM32F446xx")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -g -O0")
    set(DEFINITIONS "${DEFINITIONS}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -O1")
ELSEIF(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -Os")
ENDIF()

#preprocesor directives
set(DEFINITIONS "${DEFINITIONS} -D${MCU} -DHSE_VALUE=25000000")

#warnings
set(WARNINGS "${WARNINGS} -Wall -Wextra -Wno-main")

#compile and link flags
set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -mcpu=${CPU} -mthumb -ffunction-sections -fdata-sections")
set(COMMON_LINK_FLAGS "${COMMON_LINK_FLAGS} -mcpu=${CPU} -mthumb -nostartfiles -Wl,--gc-sections")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILE_FLAGS} ${DEFINITIONS} ${WARNINGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_LINK_FLAGS}")

#linker script
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/app/linker.ld)

#source code
add_executable(${TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/app/board.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/generator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/startup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/tests.c
)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/inc/)

#libraries for MCU and drivers
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/cmsis)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/driver_mcu)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/driver_dev)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/debug)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/utillib/src/core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/utillib/src/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mcu_libs/src/utillib/src/cli)

target_link_libraries(${TARGET_NAME} PRIVATE cmsis driver_mcu driver_dev debugLib utillib-cli)

#linker script
target_link_options(${TARGET_NAME} PRIVATE -Wl,-T ${LINKER_SCRIPT})
set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

#fancy magic
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_NAME} ${TARGET_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex --gap-fill 0 ${TARGET_NAME} ${TARGET_NAME}.hex
    COMMAND ${CMAKE_SIZE} ${TARGET_NAME}
)
