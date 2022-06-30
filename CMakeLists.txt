cmake_minimum_required(VERSION 3.13)

set(NAME picoadb)
set(DESC "PicoADB")
set(VERSION "0.1")
set(URL "https://github.com/dcooperdalrymple/PicoADB")

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

# rest of your project
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(./include)
add_executable(${NAME}
    src/adb.cpp
    src/hid.cpp
    main.cpp
    src/usb_descriptors.c
)

target_link_libraries(${NAME} pico_stdlib hardware_gpio hardware_uart hardware_irq hardware_sync tinyusb_device tinyusb_board)

pico_set_program_name(${NAME} ${DESC})
pico_set_program_version(${NAME} ${VERSION})
pico_set_program_url(${NAME} ${URL})

pico_add_extra_outputs(${NAME})
