set(CEF_VERSION "143.0.10+g8aed01b+chromium-143.0.7499.110")
set(CEF_PLATFORM "")

if(WIN32)
    set(CEF_PLATFORM "windows64")
    set(CEF_LIB_NAME "libcef_dll_wrapper.lib")
    set(CEF_LIB_SUBDIR "libcef_dll_wrapper/Release/")
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(CEF_PLATFORM "macosarm64")
    else()
        set(CEF_PLATFORM "macosx64")
    endif()
    set(CEF_LIB_NAME "libcef_dll_wrapper.a")
    set(CEF_LIB_SUBDIR "libcef_dll_wrapper/")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CEF_PLATFORM "linux64")
    set(CEF_LIB_NAME "libcef_dll_wrapper.a")
    set(CEF_LIB_SUBDIR "libcef_dll_wrapper/")
else()
    message(FATAL_ERROR "Unsupported platform for CEF")
endif()

set(CEF_URL "https://cef-builds.spotifycdn.com/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}_minimal.tar.bz2")

set(_cef_install_script "${CMAKE_CURRENT_BINARY_DIR}/cef_install.cmake")
file(WRITE "${_cef_install_script}" "
file(MAKE_DIRECTORY \"${DESTDIR}/include/cef\")
file(COPY \"\${SOURCE_DIR}/include/\" DESTINATION \"${DESTDIR}/include/cef\")
execute_process(
    COMMAND \${CMAKE_COMMAND} -E create_symlink . \"${DESTDIR}/include/cef/include\"
    RESULT_VARIABLE symlink_result
)
if(NOT symlink_result EQUAL 0)
    message(WARNING \"Failed to create CEF include symlink\")
endif()

file(MAKE_DIRECTORY \"${DESTDIR}/lib\")
file(COPY \"\${BINARY_DIR}/${CEF_LIB_SUBDIR}${CEF_LIB_NAME}\" DESTINATION \"${DESTDIR}/lib\")

file(MAKE_DIRECTORY \"${DESTDIR}/lib/cef/Release\")
file(COPY \"\${SOURCE_DIR}/Release/\" DESTINATION \"${DESTDIR}/lib/cef/Release\")

if(EXISTS \"\${SOURCE_DIR}/Resources\")
    file(MAKE_DIRECTORY \"${DESTDIR}/lib/cef/Resources\")
    file(COPY \"\${SOURCE_DIR}/Resources/\" DESTINATION \"${DESTDIR}/lib/cef/Resources\")
endif()
")

if(APPLE)
    file(APPEND "${_cef_install_script}" "
file(MAKE_DIRECTORY \"${DESTDIR}/lib/cef/Frameworks\")
file(COPY \"\${SOURCE_DIR}/Release/Chromium Embedded Framework.framework\"
     DESTINATION \"${DESTDIR}/lib/cef/Frameworks\")
")
endif()

ExternalProject_Add(
    dep_CEF
    EXCLUDE_FROM_ALL    ON
    INSTALL_DIR         ${DESTDIR}
    DOWNLOAD_DIR        ${DEP_DOWNLOAD_DIR}/CEF
    URL                 ${CEF_URL}
    CONFIGURE_COMMAND   ${CMAKE_COMMAND}
            -S <SOURCE_DIR>
            -B <BINARY_DIR>
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${DESTDIR}
            -DCMAKE_PREFIX_PATH=${DESTDIR}
            -DBUILD_SHARED_LIBS=OFF
    BUILD_COMMAND       ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target libcef_dll_wrapper
    INSTALL_COMMAND     ${CMAKE_COMMAND}
            -DSOURCE_DIR=<SOURCE_DIR>
            -DBINARY_DIR=<BINARY_DIR>
            -P "${_cef_install_script}"
)

if(MSVC)
    add_debug_dep(dep_CEF)
endif()
