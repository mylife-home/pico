add_library(common INTERFACE)

target_include_directories(common
    INTERFACE
    .)

target_sources(common
    INTERFACE
    application.cc
    scheduler.cc
    logger.cc
    status_blink.cc
    internal_temp.cc
    shell.cc
)

target_link_libraries(common
    INTERFACE
)
