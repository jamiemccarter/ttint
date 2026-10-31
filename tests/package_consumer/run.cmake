# Runs the built consumer and checks its output. Output is captured, so it is not a
# terminal and render_ansi must emit the bare text with no escape codes.
find_program(consumer ttint_package_consumer
    PATHS ${CONSUMER_DIR}/${CONFIG} ${CONSUMER_DIR}
    NO_DEFAULT_PATH REQUIRED)
execute_process(COMMAND ${consumer} OUTPUT_VARIABLE output RESULT_VARIABLE status)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "consumer exited with ${status}")
endif()
if(NOT output STREQUAL "installed package\n")
    message(FATAL_ERROR "unexpected consumer output: [${output}]")
endif()
