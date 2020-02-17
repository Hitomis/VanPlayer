cmake_minimum_required(VERSION 3.4.1)


#添加头文件路径（相对于本文件路径）
include_directories(src/main/cpp/include)

##设置ffmpeg库所在路径的变量
set(FF ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FF}/libavcodec.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FF}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FF}/libavutil.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FF}/libswscale.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FF}/libswresample.so)

add_library(
        native-lib
        SHARED
        src/main/cpp/XLog.cpp
        src/main/cpp/IDemux.cpp
        src/main/cpp/VanDemux.cpp
        src/main/cpp/XData.cpp
        src/main/cpp/XParameter.cpp
        src/main/cpp/XThread.cpp
        src/main/cpp/IObserver.cpp
        src/main/cpp/IDecode.cpp
        src/main/cpp/VanDecode.cpp
        src/main/cpp/IVideoView.cpp
        src/main/cpp/VanVideoView.cpp
        src/main/cpp/XTexture.cpp
        src/main/cpp/XShader.cpp
        src/main/cpp/XEGL.cpp
        src/main/cpp/IResample.cpp
        src/main/cpp/VanResample.cpp
        src/main/cpp/IAudioPlay.cpp
        src/main/cpp/VanAudioPlay.cpp
        src/main/cpp/native-lib.cpp)

find_library(log-lib log)

target_link_libraries(
        native-lib
        GLESv2
        EGL
        OpenSLES
        android
        avcodec avformat avutil swscale swresample
        ${log-lib})