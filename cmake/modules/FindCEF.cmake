find_path(CEF_INCLUDE_DIR
    NAMES cef_app.h
    PATHS ${CMAKE_PREFIX_PATH}/include/cef
    PATH_SUFFIXES include
)

if(WIN32)
    set(CEF_WRAPPER_LIB_NAME "libcef_dll_wrapper.lib")
    set(CEF_LIBRARY_NAME "libcef.lib")
    set(CEF_DLL_NAME "libcef.dll")
elseif(APPLE)
    set(CEF_WRAPPER_LIB_NAME "libcef_dll_wrapper.a")
    set(CEF_LIBRARY_NAME "")
elseif(UNIX)
    set(CEF_WRAPPER_LIB_NAME "libcef_dll_wrapper.a")
    set(CEF_LIBRARY_NAME "libcef.so")
endif()

find_library(CEF_WRAPPER_LIBRARY
    NAMES ${CEF_WRAPPER_LIB_NAME}
    PATHS ${CMAKE_PREFIX_PATH}/lib
    PATH_SUFFIXES lib
)

if(WIN32 OR UNIX AND NOT APPLE)
    find_library(CEF_LIBRARY
        NAMES ${CEF_LIBRARY_NAME}
        PATHS ${CMAKE_PREFIX_PATH}/lib/cef/Release
        PATH_SUFFIXES lib/cef/Release
        NO_DEFAULT_PATH
    )
endif()

if(NOT APPLE)
    find_path(CEF_RESOURCE_DIR
        NAMES icudtl.dat
        PATHS ${CMAKE_PREFIX_PATH}/lib/cef/Resources
        PATH_SUFFIXES lib/cef/Resources
        NO_DEFAULT_PATH
    )
endif()

if(APPLE)
    find_path(CEF_FRAMEWORK_DIR
        NAMES "Chromium Embedded Framework.framework"
        PATHS ${CMAKE_PREFIX_PATH}/lib/cef/Frameworks
        PATH_SUFFIXES lib/cef/Frameworks
        NO_DEFAULT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)

if(APPLE)
    find_package_handle_standard_args(CEF
        REQUIRED_VARS CEF_INCLUDE_DIR CEF_WRAPPER_LIBRARY CEF_FRAMEWORK_DIR
    )
elseif(WIN32 OR UNIX)
    find_package_handle_standard_args(CEF
        REQUIRED_VARS CEF_INCLUDE_DIR CEF_WRAPPER_LIBRARY CEF_LIBRARY CEF_RESOURCE_DIR
    )
endif()

if(CEF_FOUND)
    set(CEF_INCLUDE_DIRS ${CEF_INCLUDE_DIR})

    if(APPLE)
        set(CEF_LIBRARIES
            ${CEF_WRAPPER_LIBRARY}
            "${CEF_FRAMEWORK_DIR}/Chromium Embedded Framework.framework"
        )
    else()
        set(CEF_LIBRARIES
            ${CEF_WRAPPER_LIBRARY}
            ${CEF_LIBRARY}
        )
    endif()

    if(NOT TARGET CEF::CEF)
        add_library(CEF::CEF INTERFACE IMPORTED)
        set_target_properties(CEF::CEF PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CEF_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${CEF_LIBRARIES}"
        )

        if(APPLE)
            set_target_properties(CEF::CEF PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "CEF_USE_SANDBOX"
            )
        endif()
    endif()

    mark_as_advanced(
        CEF_INCLUDE_DIR
        CEF_WRAPPER_LIBRARY
        CEF_LIBRARY
        CEF_RESOURCE_DIR
        CEF_FRAMEWORK_DIR
    )
endif()
