cmake_minimum_required(VERSION 3.10)
project(ffmpeg_learn)

set(CMAKE_CXX_STANDARD 11)

set(INC_OPENGL_DIR /Users/xmly/Desktop/workplace/ffmpeg_learn/include)
set(GLEW_H /usr/local/Cellar/glew/2.1.0/include/GL)
set(GLFW_H /usr/local/Cellar/glfw/3.3/include/GLFW)
set(ASSIMP_H /usr/local/Cellar/assimp/4.1.0/include)
set(GLAD_H /usr/local/include/glad)
set(KH_H /usr/local/include/KHR)

set(GLEW_LINK /usr/local/Cellar/glew/2.1.0/lib/libGLEW.2.1.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3/lib/libglfw.3.3.dylib)
set(ASSIMP_LINK /usr/local/Cellar/assimp/4.1.0/lib/libassimp.4.1.0.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK} ${ASSIMP_LINK})

include_directories(${GLEW_H} ${GLFW_H} ${GLAD_H} ${KH_H} ${ASSIMP_H})
include_directories(${INC_OPENGL_DIR})

set(INC_FFMPEG_DIR /usr/local/Cellar/ffmpeg/4.2.2_1/include)
set(FFMPEG_CODEC /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libavcodec.dylib)
set(FFMPEG_FILTER /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libavfilter.dylib)
set(FFMPEG_FORMAT /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libavformat.dylib)
set(FFMPEG_UTIL /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libavutil.dylib)
set(FFMPEG_SCALE /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libswscale.dylib)
set(FFMPEG_DEVICE /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libavdevice.dylib)
set(FFMPEG_SAMPLE /usr/local/Cellar/ffmpeg/4.2.2_1/lib/libswresample.dylib)
link_libraries(${FFMPEG} ${FFMPEG_CODEC} ${FFMPEG_FILTER} ${FFMPEG_FORMAT}
        ${FFMPEG_UTIL} ${FFMPEG_SCALE} ${FFMPEG_DEVICE} ${FFMPEG_SAMPLE})

include_directories(${INC_FFMPEG_DIR})
#add_executable(ffmpeg_learn libsrc/glad.c cpp/H264Decoder.cpp cpp/H264Decoder.h cpp/opengl/OpenGlWindow.cpp cpp/opengl/OpenGlWindow.h)
#add_executable(ffmpeg_learn  cpp/PCMToMp3Decoder.cpp cpp/PCMToMp3Decoder.h cpp/Mp4Synthesis.cpp cpp/Mp4Synthesis.h)
#add_executable(ffmpeg_learn  cpp/AACToPCMDecoder.cpp cpp/AACToPCMDecoder.h)
#add_executable(ffmpeg_learn  main.cpp cpp/h264/Base.cpp main.cpp cpp/h264/Base.h main.cpp cpp/h264/Derived.cpp cpp/h264/Derived.h)
#add_executable(ffmpeg_learn  cpp/Mp4Synthesis.cpp cpp/Mp4Synthesis.h)
add_executable(ffmpeg_learn  cpp/YUVToH264.cpp cpp/YUVToH264.h)
#判断是否是苹果电脑
if(APPLE)
    #target_link_libraries(ffmpeg_learn ${LINK_DIR}) #这样写会有错误
    target_link_libraries(ffmpeg_learn "-framework OpenGL")
    target_link_libraries(ffmpeg_learn "-framework GLUT")
endif()
