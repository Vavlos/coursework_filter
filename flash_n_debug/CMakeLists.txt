message(CHECK_START "\tFlash and debug tools")


set(PROGRAMMER stlink)

# Openocd target config selection (incomplete)
if(DEVICE MATCHES "stm32f0")
    set(TARGET_DEVICE stm32f0x)
elseif(DEVICE MATCHES "stm32f1")
    set(TARGET_DEVICE stm32f1x)
elseif(DEVICE MATCHES "stm32f2")
    set(TARGET_DEVICE stm32f2x)
elseif(DEVICE MATCHES "stm32f3")
    set(TARGET_DEVICE stm32f3x)
elseif(DEVICE MATCHES "stm32f4")
    set(TARGET_DEVICE stm32f4x)
elseif(DEVICE MATCHES "stm32f7")
    set(TARGET_DEVICE stm32f7x)
elseif(DEVICE MATCHES "stm32g0")
    set(TARGET_DEVICE stm32g0x)
elseif(DEVICE MATCHES "stm32g4")
    set(TARGET_DEVICE stm32g4x)
elseif(DEVICE MATCHES "stm32h7")
    set(TARGET_DEVICE stm32h7x)
elseif(DEVICE MATCHES "stm32l4")
    set(TARGET_DEVICE stm32l4x)
elseif(DEVICE MATCHES "stm32l5")
    set(TARGET_DEVICE stm32l5x)
else()
    message(WARNING "\tOpenocd target: unknown target!")
endif()

# Openocd interface and target strings forming
string(APPEND TARGET_DEVICE ".cfg")
string(APPEND PROGRAMMER ".cfg")
string(PREPEND TARGET_DEVICE "target/")
string(PREPEND PROGRAMMER "interface/")

message(VERBOSE "\tOpenocd target: " ${TARGET_DEVICE})


#===============================================================================
add_custom_target(flash
    COMMAND openocd -f ${PROGRAMMER} -f ${TARGET_DEVICE} -c init -c "reset halt" -c "program ${CMAKE_BINARY_DIR}/firmware" -c reset -c shutdown 
    DEPENDS ${PROJECT_NAME}
    COMMENT "Start flashing firmware"
)


#===============================================================================
# Install procedure used for flashing firmware
set(OOCD_FLASH_SCRIPT
     "MESSAGE(STATUS \"Flashing firmware\")
     execute_process (
          COMMAND openocd
               -f ${PROGRAMMER}
               -f ${TARGET_DEVICE}
               -c \"init\" 
               -c \"reset halt\"
               -c \"program ${CMAKE_BINARY_DIR}/firmware\" 
               -c \"reset\"
               -c \"shutdown\"
          RESULT_VARIABLE res
     )
     if(res AND NOT res EQUAL 0)
         message(FATAL_ERROR \"Flashing failed!\")
     endif()
     MESSAGE(STATUS \"Flashing complete\")
     "
)

install(CODE ${OOCD_FLASH_SCRIPT})


message(CHECK_PASS "Done")
