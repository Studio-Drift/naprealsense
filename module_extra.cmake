if(NOT TARGET realsense)
    find_package(realsense REQUIRED)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${REALSENSE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${REALSENSE_LIBS})

if(WIN32)
    # Copy realsense DLL to build directory on Windows (plus into packaged app)
    copy_realsense_dll()
endif()

if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    if(UNIX)
        install(DIRECTORY ${REALSENSE_LIBS_DIR}/lib/ DESTINATION lib)
    endif()
endif()