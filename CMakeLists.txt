# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

# Chip: rp2040 (Pico) or rp2350 (Pico 2)
#set(PICO_PLATFORM "rp2040" CACHE STRING "Chip type")
set(PICO_PLATFORM "rp2350" CACHE STRING "Chip type")

# Board: murmulator (MURMULATOR 1.x), murmulator2 (MURMULATOR 2.0),
# olimex-pico-pc (Olimex RP2040-PICO-PC) or waveshare_rp2350_pizero
#set(PICO_BOARD "murmulator" CACHE STRING "Board type")
#set(PICO_BOARD "olimex-pico-pc" CACHE STRING "Board type")
#set(PICO_BOARD "waveshare_rp2350_pizero" CACHE STRING "Board type")
set(PICO_BOARD "murmulator2" CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(ngpc C CXX ASM)
SET(BUILD_NAME "${PROJECT_NAME}")

option(I2S "Enable I2S sound" ON)
option(I2S_CS4334 "Enable I2S CS4334 sound" OFF)
option(VGA "Enable VGA" OFF)
option(TFT "Enable TFT display" OFF)
option(ILI9341 "Enable TFT ILI9341 display" OFF)
option(HDMI "Enable HDMI display" ON)
option(TV "Enable TV composite output" OFF)
option(SOFTTV "Enable TV soft composite output" OFF)

if (${PICO_BOARD} MATCHES "waveshare_rp2350_pizero")
	# Waveshare RP2350-PiZero (z0p2): RP2350B, 16 MB flash, HDMI on GPIO32..39
	set(HDMI ON CACHE BOOL "Enable HDMI display" FORCE)
endif()


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#add_compile_options(-flto)
#set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -funroll-loops -fno-jump-tables  -ffast-math -feliminate-unused-debug-types -ffunction-sections -fdata-sections -Ofast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -fno-jump-tables -fno-rtti  -ffast-math -feliminate-unused-debug-types -ffunction-sections -fdata-sections -Ofast")

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

pico_sdk_init()

add_subdirectory(drivers/audio)

add_subdirectory(drivers/ps2kbd)
add_subdirectory(drivers/nespad)
add_subdirectory(drivers/fatfs)
add_subdirectory(drivers/sdcard)

add_subdirectory(drivers/vga-nextgen)
add_subdirectory(drivers/st7789)
add_subdirectory(drivers/hdmi)
add_subdirectory(drivers/tv)
add_subdirectory(drivers/tv-software)

add_subdirectory(drivers/graphics)

# INCLUDE FILES THAT SHOULD BE COMPILED:
file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

message(STATUS "Add source files:")
foreach (SRC_FILE IN LISTS SRC)
	message(STATUS "${SRC_FILE}")
endforeach ()
message(STATUS "")

add_executable(${PROJECT_NAME} ${SRC})

if (${PICO_PLATFORM} MATCHES "rp2040")
	pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
	target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
	pico_set_boot_stage2(${PROJECT_NAME} slower_boot2)
	if (${PICO_BOARD} MATCHES "olimex-pico-pc")
		SET(BUILD_NAME "PCp1-${PROJECT_NAME}")
	else()
		SET(BUILD_NAME "m1p1-${PROJECT_NAME}")
	endif()
else()
	if (${PICO_BOARD} MATCHES "olimex-pico-pc")
		SET(BUILD_NAME "PCp2-${PROJECT_NAME}")
	elseif (${PICO_BOARD} MATCHES "waveshare_rp2350_pizero")
		SET(BUILD_NAME "z0p2-${PROJECT_NAME}")
	elseif (${PICO_BOARD} MATCHES "murmulator2")
		SET(BUILD_NAME "m2p2-${PROJECT_NAME}")
	else()
		SET(BUILD_NAME "m1p2-${PROJECT_NAME}")
	endif()
endif()

# RP2350: run the NeoPop core (TLCS-900h, Z80, memory, graphics, sound) from
# SRAM instead of XIP flash.  The cartridge itself is read through XIP, and
# code fetches competing with it for the 16 KB XIP cache made the emulation
# too slow.  The SDK linker script already keeps libgcc/libm in RAM through
# EXCLUDE_FILE lists; neopop objects are added to them, so their .text and
# .rodata fall through to the RAM-resident .data section.
if (${PICO_PLATFORM} MATCHES "rp2350")
	# The pattern must match object names on every host: Windows builds give
	# "CMakeFiles\ngpc.dir\src\neopop\mem.c.obj", Linux ".../src/neopop/mem.c.o".
	file(READ ${PICO_SDK_PATH}/src/rp2_common/pico_crt0/rp2350/memmap_default.ld NGPC_LDS)
	set(NGPC_LDS_EXCLUDE "EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:)")
	string(FIND "${NGPC_LDS}" "${NGPC_LDS_EXCLUDE}" NGPC_LDS_POS)
	if (NGPC_LDS_POS EQUAL -1)
		message(FATAL_ERROR "memmap_default.ld of this pico-sdk has no '${NGPC_LDS_EXCLUDE}': cannot move the NeoPop core to RAM")
	endif()
	string(REPLACE "${NGPC_LDS_EXCLUDE}"
	               "EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a: *neopop*)"
	               NGPC_LDS "${NGPC_LDS}")
	file(WRITE ${CMAKE_BINARY_DIR}/memmap_ngpc.ld "${NGPC_LDS}")
	message(STATUS "NeoPop core runs from RAM: ${CMAKE_BINARY_DIR}/memmap_ngpc.ld")
	pico_set_linker_script(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/memmap_ngpc.ld)
endif()

if (${PICO_BOARD} MATCHES "olimex-pico-pc")
	pico_set_program_name(ngpc "ngpc emulator by xrip for Olimex RP2040-PICO-PC")
elseif (${PICO_BOARD} MATCHES "waveshare_rp2350_pizero")
	pico_set_program_name(ngpc "ngpc emulator by xrip for Waveshare RP2350-PiZero")
else()
	pico_set_program_name(ngpc "ngpc emulator by xrip for MURMULATOR devboard")
endif()
pico_set_program_version(ngpc "develop")

target_include_directories(${PROJECT_NAME} PRIVATE src src/neopop)

IF(TFT)
	target_link_libraries(${PROJECT_NAME} PRIVATE st7789)
	target_compile_definitions(${PROJECT_NAME} PRIVATE TFT)
	SET(BUILD_NAME "${BUILD_NAME}-TFT")
	IF(ILI9341)
		SET(BUILD_NAME "${BUILD_NAME}-ILI9341")
		target_compile_definitions(${PROJECT_NAME} PRIVATE ILI9341)
	ELSE()
		SET(BUILD_NAME "${BUILD_NAME}-ST7789")
	ENDIF()
ELSEIF(HDMI)
	target_link_libraries(${PROJECT_NAME} PRIVATE hdmi)
	target_compile_definitions(${PROJECT_NAME} PRIVATE HDMI)
	SET(BUILD_NAME "${BUILD_NAME}-HDMI")
ELSEIF(TV)
	target_compile_definitions(${PROJECT_NAME} PRIVATE TV)
	target_link_libraries(${PROJECT_NAME} PRIVATE tv)
	SET(BUILD_NAME "${BUILD_NAME}-TV")
ELSEIF(SOFTTV)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SOFTTV)
	target_link_libraries(${PROJECT_NAME} PRIVATE tv-software)
	SET(BUILD_NAME "${BUILD_NAME}-TV-SOFT")
ELSE()

	target_compile_definitions(${PROJECT_NAME} PRIVATE VGA)
	target_link_libraries(${PROJECT_NAME} PRIVATE vga-nextgen)
	SET(BUILD_NAME "${BUILD_NAME}-VGA")
ENDIF()

IF(NOT I2S)
	if (${PICO_BOARD} MATCHES "murmulator2")
		# PWM on GP10/GP11 (one PWM slice)
		target_compile_definitions(${PROJECT_NAME} PRIVATE AUDIO_PWM_PIN=10)
		SET(BUILD_NAME "${BUILD_NAME}-PWM")
	else()
		target_compile_definitions(${PROJECT_NAME} PRIVATE AUDIO_PWM_PIN=26)
	endif()
ELSEIF(I2S_CS4334)
	target_compile_definitions(${PROJECT_NAME} PRIVATE I2S_CS4334)
	SET(BUILD_NAME "${BUILD_NAME}-I2S-CS4334")
ELSE()
	SET(BUILD_NAME "${BUILD_NAME}-I2S-TDA1387")
ENDIF()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${BUILD_NAME}")

target_link_libraries(${PROJECT_NAME} PRIVATE
		graphics
		audio

		nespad
		sdcard
		ps2kbd
		fatfs

		pico_runtime
        pico_stdlib
        pico_multicore
        hardware_dma
        hardware_pio
        hardware_i2c
        hardware_interp
        hardware_timer
        hardware_clocks
        hardware_pwm
        hardware_flash

		tinyusb_host tinyusb_board
)

# Pins and flash size come from boards/${PICO_BOARD}.h
target_compile_definitions(${PROJECT_NAME} PRIVATE
		USE_PS2_KBD
		USE_NESPAD
		SMS_SINGLE_FILE=1
)
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND picotool info ${OUTPUT_DIR}/$<TARGET_PROPERTY:OUTPUT_NAME>.uf2 &&
#            COMMAND picotool reboot -f -u & mode COM27 baud=12 parity=n data=8 stop=1
#            picotool load ${OUTPUT_DIR}/$<TARGET_PROPERTY:OUTPUT_NAME>.uf2 &&
#            picotool reboot
#    )


target_link_options(${PROJECT_NAME} PRIVATE -Xlinker --print-memory-usage --data-sections --function-sections)
pico_add_extra_outputs(${PROJECT_NAME})



