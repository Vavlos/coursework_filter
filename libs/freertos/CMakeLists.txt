message(CHECK_START "\tFreeRTOS-kernel")


#===============================================================================
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM
    INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(freertos_config
    INTERFACE
    projCOVERAGE_TEST=0
)

target_link_libraries(freertos_config
    INTERFACE
    libopencm3
)


#===============================================================================
if(NOT FREERTOS_KERNEL_REPOSITORY)
  set(FREERTOS_KERNEL_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git)
  message(NOTICE "\t\tSelect default FreeRTOS-kernel repository.")
endif()

if(NOT FREERTOS_KERNEL_TAG)
  set(FREERTOS_KERNEL_TAG V11.1.0)
  message(NOTICE "\t\tSelect default FreeRTOS-kernel tag.")
endif()

if(NOT FREERTOS_CONFIG_FILE_DIRECTORY)
  set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_SOURCE_DIR}/src CACHE STRING "")
  message(NOTICE "\t\tSelect default FreeRTOSConfig.h directory.")
endif()

if(NOT FREERTOS_PORT)
  set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")
  message(NOTICE "\t\tSelect default FreeRTOS port.")
endif()

if(NOT FREERTOS_HEAP)
  set(FREERTOS_HEAP "4" CACHE STRING "")
  message(NOTICE "\t\tSelect default heap model.")
endif()

message(NOTICE "\t\tUse repository: " ${FREERTOS_KERNEL_REPOSITORY})
message(NOTICE "\t\tUse tag: " ${FREERTOS_KERNEL_TAG})
message(NOTICE "\t\tFreeRTOSConfig.h directory: " ${FREERTOS_CONFIG_FILE_DIRECTORY})
message(NOTICE "\t\tSelected FreeRTOS port: " ${FREERTOS_PORT})
message(NOTICE "\t\tSelected heap model: " ${FREERTOS_HEAP})


include(FetchContent)

FetchContent_Declare(freertoskernel_repo
  GIT_REPOSITORY ${FREERTOS_KERNEL_REPOSITORY}
  GIT_TAG V11.1.0
)

FetchContent_MakeAvailable(freertoskernel_repo)

target_link_libraries(freertos_kernel
    PRIVATE
    libopencm3
)


message(CHECK_PASS "Done")
