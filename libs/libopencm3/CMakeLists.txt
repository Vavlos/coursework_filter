message(CHECK_START "\tLibopencm3")


if(NOT DEVICE)
  message(FATAL_ERROR "\t\tDevice must be defined at project level!\tExample: set(DEVICE stm32f407vg).")
endif()


#===============================================================================
include(FetchContent)
FetchContent_Declare(llibopencm3
  GIT_REPOSITORY https://github.com/libopencm3/libopencm3
  GIT_TAG master
)

FetchContent_MakeAvailable(llibopencm3)
FetchContent_GetProperties(llibopencm3)


include(libopencm3_query.cmake) # Query defines and parameters based on DEVICE variable


message(NOTICE "\t\tRun libopencm3 make")

execute_process(COMMAND make TARGETS=${LOCM3_TARGET}
    WORKING_DIRECTORY ${llibopencm3_SOURCE_DIR}
)


#===============================================================================
add_library(libopencm3 INTERFACE)

target_compile_definitions(libopencm3 INTERFACE ${LOCM3_DEVICE_DEFINES})
target_compile_options(libopencm3 INTERFACE ${MCPU} ${MTHUMB} ${MFPU} ${MFLOAT_ABI})
target_include_directories(libopencm3 INTERFACE  ${LOCM3_INCLUDE_DIR})
target_link_libraries(libopencm3 INTERFACE ${LOCM3_STATIC_LIB_PREFIX}${LOCM3_DEVICE_SERIE}.a)
target_link_options(libopencm3 INTERFACE -T${LOCM3_LINKER_GENERATED} ${MCPU} ${MTHUMB} ${MFPU} ${MFLOAT_ABI})


message(CHECK_PASS "Done")
