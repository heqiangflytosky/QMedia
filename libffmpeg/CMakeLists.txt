
cmake_minimum_required(VERSION 3.4.1)

# 添加头文件路径
include_directories(src/main/cpp/include)

add_library( # Specifies the name of the library.
        native-lib
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        src/main/cpp/qplayer.cpp
        src/main/cpp/AudioPlayer.cpp
        src/main/cpp/AudioChannel.cpp
        src/main/cpp/BasePlayer.cpp
        src/main/cpp/FFmpeg.cpp
        src/main/cpp/JavaCall.cpp
        src/main/cpp/PlayStatus.cpp
        src/main/cpp/QQueue.cpp
        src/main/cpp/VideoPlayer.cpp)

#添加libavcodec-57.so
add_library( avcodec-57
        SHARED
        IMPORTED)
set_target_properties( avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)


#添加libavdevice-57.so
add_library( avdevice-57
        SHARED
        IMPORTED)
set_target_properties( avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-57.so)

add_library( avfilter-6
        SHARED
        IMPORTED)
set_target_properties( avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)

add_library( avformat-57
        SHARED
        IMPORTED)
set_target_properties( avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)

add_library( avutil-55
        SHARED
        IMPORTED)
set_target_properties( avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)

add_library( swresample-2
        SHARED
        IMPORTED)
set_target_properties( swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)

add_library( swscale-4
        SHARED
        IMPORTED)
set_target_properties( swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)

add_library( postproc-54
        SHARED
        IMPORTED)
set_target_properties( postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-54.so)


# 连接FFmpeg动态库
target_link_libraries( # Specifies the target library.
        native-lib
        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        swresample-2
        swscale-4
        postproc-54
        OpenSLES
        android
        log
        )