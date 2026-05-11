#================================================================================
cmake_minimum_required(VERSION 3.25)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)


#================================================================================
project(coursework LANGUAGES CXX C ASM)
set(DEVICE stm32f407vg)
# Поддерживаемые семейства МК см. в файле flsh_n_debug/CMakeLists.txt


#================================================================================
add_subdirectory(libs/libopencm3)

add_subdirectory(src)

add_subdirectory(flash_n_debug)

add_subdirectory(doc)

