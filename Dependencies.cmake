# Deliberately uses CMake 3.10 APIs (no FetchContent).
set(QREADER_DEPENDENCY_DIR "${CMAKE_BINARY_DIR}/dependencies" CACHE PATH
    "Directory for downloaded and extracted dependencies")

function(qreader_archive name url hash folder)
    set(cache "${QREADER_DEPENDENCY_DIR}/downloads")
    set(sources "${QREADER_DEPENDENCY_DIR}/sources")
    file(MAKE_DIRECTORY "${cache}" "${sources}")
    set(archive "${cache}/${name}.tar.gz")
    if(EXISTS "${archive}")
        file(SHA256 "${archive}" actual)
        if(NOT actual STREQUAL hash)
            message(FATAL_ERROR "SHA256 mismatch: ${archive}. Remove it and configure again.")
        endif()
    else()
        message(STATUS "Downloading ${name}")
        file(DOWNLOAD "${url}" "${archive}.partial" EXPECTED_HASH "SHA256=${hash}"
            TLS_VERIFY ON STATUS status SHOW_PROGRESS)
        list(GET status 0 code)
        if(NOT code EQUAL 0)
            file(REMOVE "${archive}.partial")
            message(FATAL_ERROR "Download failed: ${status}")
        endif()
        file(RENAME "${archive}.partial" "${archive}")
    endif()
    if(NOT EXISTS "${sources}/${folder}/CMakeLists.txt")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf "${archive}"
            WORKING_DIRECTORY "${sources}" RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Cannot extract ${archive}")
        endif()
    endif()
    set(${name}_SOURCE "${sources}/${folder}" PARENT_SCOPE)
endfunction()

function(qreader_dependencies)
    qreader_archive(opencv-4.5.5 "https://codeload.github.com/opencv/opencv/tar.gz/refs/tags/4.5.5"
        a1cfdcf6619387ca9e232687504da996aaa9f7b5689986b8331ec02cb61d28ad opencv-4.5.5)
    qreader_archive(zxing-1.4.0 "https://codeload.github.com/zxing-cpp/zxing-cpp/tar.gz/refs/tags/v1.4.0"
        126767bb56f8a1f25ae84d233db2e9b9be50d71f5776092d0e170ca0f0ed1862 zxing-cpp-1.4.0)

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_LIST "core,imgproc,imgcodecs,objdetect" CACHE STRING "" FORCE)
    foreach(option BUILD_TESTS BUILD_PERF_TESTS BUILD_EXAMPLES BUILD_DOCS BUILD_JAVA BUILD_opencv_apps
            BUILD_opencv_python2 BUILD_opencv_python3 BUILD_opencv_js BUILD_PACKAGE BUILD_IPP_IW
            WITH_IPP WITH_ITT WITH_OPENCL WITH_OPENEXR WITH_WEBP WITH_TIFF WITH_JASPER WITH_OPENJPEG
            WITH_FFMPEG WITH_GSTREAMER WITH_MSMF WITH_DSHOW WITH_VFW WITH_WIN32UI WITH_QT WITH_GTK
            WITH_TBB WITH_OPENMP WITH_CONCURRENCY WITH_PTHREADS_PF WITH_LAPACK WITH_EIGEN
            WITH_QUIRC OPENCV_ENABLE_NONFREE OPENCV_GENERATE_PKGCONFIG)
        set(${option} OFF CACHE BOOL "" FORCE)
    endforeach()
    foreach(option WITH_JPEG WITH_PNG BUILD_JPEG BUILD_PNG BUILD_ZLIB WITH_PROTOBUF BUILD_PROTOBUF)
        set(${option} ON CACHE BOOL "" FORCE)
    endforeach()
    set(CPU_BASELINE SSE2 CACHE STRING "" FORCE)
    set(CPU_DISPATCH "" CACHE STRING "" FORCE)
    set(OPENCV_DOWNLOAD_PATH "${QREADER_DEPENDENCY_DIR}/downloads" CACHE PATH "" FORCE)
    set(CMAKE_CXX_STANDARD 11)
    add_subdirectory("${opencv-4.5.5_SOURCE}" "${CMAKE_BINARY_DIR}/deps/opencv" EXCLUDE_FROM_ALL)
    if(NOT TARGET opencv_objdetect)
        message(FATAL_ERROR "OpenCV objdetect and its transitive dependencies must be enabled.")
    endif()
    set(QREADER_OPENCV_INCLUDES "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/deps/opencv"
        "${opencv-4.5.5_SOURCE}/modules/core/include" "${opencv-4.5.5_SOURCE}/modules/imgproc/include"
        "${opencv-4.5.5_SOURCE}/modules/imgcodecs/include" "${opencv-4.5.5_SOURCE}/modules/objdetect/include" PARENT_SCOPE)

    # The 1.4.0 core's own entry point supports 3.10; the unused wrappers need 3.14.
    set(CMAKE_CXX_STANDARD 17)
    set(PROJECT_VERSION 1.4.0)
    set(PROJECT_VERSION_MAJOR 1)
    set(PROJECT_VERSION_MINOR 4)
    set(PROJECT_VERSION_PATCH 0)
    set(BUILD_READERS ON)
    set(BUILD_WRITERS ${QREADER_BUILD_TESTS})
    set(BUILD_UNIT_TESTS OFF)
    # 1.4 contains LGPL text codecs. Keep the library replaceable in binary distributions.
    set(BUILD_SHARED_LIBS ON)
    add_subdirectory("${zxing-1.4.0_SOURCE}/core" "${CMAKE_BINARY_DIR}/deps/zxing" EXCLUDE_FROM_ALL)
    # 3.10 otherwise appends -std=gnu++11 after the application's explicit C++20
    # flag when consuming ZXing's cxx_std_11 interface. Our targets already
    # require C++20; ZXing itself retains its PRIVATE cxx_std_17 requirement.
    set_property(TARGET ZXing PROPERTY INTERFACE_COMPILE_FEATURES "")
    set_target_properties(ZXing PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    if(MINGW)
        set_property(TARGET ZXing APPEND_STRING PROPERTY LINK_FLAGS " -static -static-libgcc -static-libstdc++ -Wl,--exclude-libs,libstdc++.a:libgcc.a:libgcc_eh.a:libpthread.a:libwinpthread.a")
    endif()

    # Include original dependency license texts with any installed binary.
    set(licenses "${CMAKE_BINARY_DIR}/licenses")
    file(MAKE_DIRECTORY "${licenses}")
    configure_file("${opencv-4.5.5_SOURCE}/LICENSE" "${licenses}/OpenCV-LICENSE.txt" COPYONLY)
    configure_file("${opencv-4.5.5_SOURCE}/3rdparty/protobuf/LICENSE" "${licenses}/Protobuf-LICENSE.txt" COPYONLY)
    configure_file("${zxing-1.4.0_SOURCE}/LICENSE" "${licenses}/ZXing-LICENSE.txt" COPYONLY)
    foreach(name NOTICE AUTHORS.ZXing LICENSE.ZXing LICENSE.Qt LGPL_EXCEPTION.Qt)
        configure_file("${zxing-1.4.0_SOURCE}/${name}" "${licenses}/ZXing-${name}" COPYONLY)
    endforeach()
    configure_file("${opencv-4.5.5_SOURCE}/3rdparty/zlib/zlib.h" "${licenses}/zlib-LICENSE-header.txt" COPYONLY)
    file(READ "${opencv-4.5.5_SOURCE}/modules/core/src/softfloat.cpp" softfloat)
    string(FIND "${softfloat}" "#include" end)
    string(SUBSTRING "${softfloat}" 0 ${end} softfloat_license)
    file(WRITE "${licenses}/SoftFloat-FDLIBM-LICENSE.txt" "${softfloat_license}")
    foreach(component libjpeg-turbo libpng zlib)
        file(GLOB texts "${opencv-4.5.5_SOURCE}/3rdparty/${component}/LICENSE*"
            "${opencv-4.5.5_SOURCE}/3rdparty/${component}/README*"
            "${opencv-4.5.5_SOURCE}/3rdparty/${component}/COPYING*")
        file(COPY ${texts} DESTINATION "${licenses}/${component}")
    endforeach()
    # Debian/Ubuntu cross-toolchain runtime notices, when available locally.
    foreach(package mingw-w64-common gcc-mingw-w64-x86-64-posix)
        if(EXISTS "/usr/share/doc/${package}/copyright")
            configure_file("/usr/share/doc/${package}/copyright" "${licenses}/${package}-copyright.txt" COPYONLY)
        endif()
    endforeach()
    if(EXISTS "/usr/share/common-licenses/GPL-3")
        configure_file("/usr/share/common-licenses/GPL-3" "${licenses}/GCC-GPL-3.txt" COPYONLY)
    endif()
    if(QREADER_BUILD_GUI)
        install(FILES "$<TARGET_FILE:ZXing>" DESTINATION .)
        install(FILES "${QREADER_DEPENDENCY_DIR}/downloads/zxing-1.4.0.tar.gz" DESTINATION sources)
    endif()
endfunction()
