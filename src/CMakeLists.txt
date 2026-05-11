message(CHECK_START "\tSources")


# Название прошивки определяется названием проекта (определено в основном сценарии cmake)
add_executable(${CMAKE_PROJECT_NAME}

    main.c
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Папки библиотек должны включаться в основной сценарии заранее
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
    libopencm3
)

set_target_properties(${CMAKE_PROJECT_NAME}
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

# Копирование готовой прошивки в папку сборки проекта для отладчика cortex-debug
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND cmake -E copy ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} ${CMAKE_BINARY_DIR}/firmware
)

#Дистрибьюция
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND arm-none-eabi-objdump -D ${CMAKE_BINARY_DIR}/firmware > ${CMAKE_BINARY_DIR}/firmware.lst
)


message(CHECK_PASS "Done")
