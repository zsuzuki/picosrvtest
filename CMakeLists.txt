cmake_minimum_required(VERSION 3.12)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_SDK_PATH "../../pico-sdk")

set(PICO_BOARD pico_w)
set(PICO_CYW43_ARCH_THREADSAFE_BACKGROUND 1)

if(DEFINED ENV{WIFI_SSID})
    set(WIFI_SSID $ENV{WIFI_SSID})
else()
    message(FATAL_ERROR "WIFI_SSID not defined")
endif()

if(DEFINED ENV{WIFI_PASSWORD})
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
else()
    message(FATAL_ERROR "WIFI_PASSWORD not defined")
endif()

add_compile_definitions(${PROJECT_NAME} PUBLIC
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    LWIP_HTTPD_CGI=1
    LWIP_HTTPD_SUPPORT_POST=1
)

include(pico_sdk_import.cmake)
project(pico_srv)

pico_sdk_init()

add_subdirectory(lib/config)
add_subdirectory(lib/lcd)
add_subdirectory(lib/font)
add_subdirectory(lib/fatfs)
add_subdirectory(lib/sdcard)

include_directories(./lib/config)
include_directories(./lib/lcd)
include_directories(./lib/font)
include_directories(./lib/fatfs)
include_directories(./lib/sdcard)
include_directories(./include)
include_directories(.)
include_directories(./lua)

set(LUASRC
    lua/lapi.c
    lua/lauxlib.c
    lua/lbaselib.c
    lua/lcode.c
    lua/lcorolib.c
    lua/lctype.c
    lua/ldblib.c
    lua/ldebug.c
    lua/ldo.c
    lua/ldump.c
    lua/lfunc.c
    lua/lgc.c
    lua/linit.c
    lua/liolib.c
    lua/llex.c
    lua/lmathlib.c
    lua/lmem.c
    lua/loadlib.c
    lua/lobject.c
    lua/lopcodes.c
    lua/loslib.c
    lua/lparser.c
    lua/lstate.c
    lua/lstring.c
    lua/lstrlib.c
    lua/ltable.c
    lua/ltablib.c
    lua/ltm.c
    lua/lundump.c
    lua/lutf8lib.c
    lua/lvm.c
    lua/lzio.c
)

# generate an executable file
add_executable(${PROJECT_NAME}
    src/draw.cpp
    src/font.cpp
    src/main.cpp
    ${LUASRC}
)

pico_set_program_name(${PROJECT_NAME} "Pico Server Test")
pico_set_program_version(${PROJECT_NAME} "0.2")

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} pico_stdlib pico_cyw43_arch_lwip_threadsafe_background pico_lwip_http lcd font config hardware_spi fatfs sdcard)
