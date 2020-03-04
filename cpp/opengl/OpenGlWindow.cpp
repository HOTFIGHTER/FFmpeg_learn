//
// Created by xmly on 2020/2/8.
//
#include <iostream>
#include "OpenGlWindow.h"
using namespace std;
OpenGlWindow::OpenGlWindow() {
    //初始化GLFW
    glfwInit();
    //指定openGl版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //mac系统需要
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //创建窗口,glfwCreateWindow函数需要窗口的宽和高作为它的前两个参数
    pWindow = glfwCreateWindow(160 * 4 * 1.5, 90 * 4 * 1.5, "FFmpeg_Learn", NULL, NULL);
    if (NULL != pWindow) {
        glfwMakeContextCurrent(pWindow);
        gladLoadGL();
        glViewport(0, 0, 640 * 1.5, 360 * 1.5);

        //顶点Sharder
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar *vertexSource = "#version 330"
                                     "\n in vec2 aTexCoord;"
                                     "\n in vec3 aPos;"
                                     "\n out vec2 aCoord;"
                                     "\n void main(){"
                                     "\n gl_Position = vec4(aPos,1.0);"
                                     "\n aCoord = aTexCoord;"
                                     "\n}"
                                     "\n";
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024] = {0};
            glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
            cout <<"vertexShader:"<<infoLog<<endl;
        }
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fragmentSource = "#version 330"
                                       "\n vec3 yuv;"
                                       "\n in vec2 aCoord;"
                                       "\n uniform sampler2D myTexture;"
                                       "\n out vec4 fragColor;"
                                       "\n void main(){"
                                       "\n yuv = texture(myTexture,aCoord).rgb;"
                                       "\n fragColor = vec4(yuv,1.0);"
                                       "\n }"
                                       "\n";
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success){
            GLchar infoLog[1024] = {0};
            glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
            printf("fragmentShader %s \n",infoLog);
        }
        mShaderProgram =  glCreateProgram();
        glAttachShader(mShaderProgram, vertexShader);
        glAttachShader(mShaderProgram, fragmentShader);
        glLinkProgram(mShaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
        if(!success){
            GLchar infoLog[1024] = {0};
            glGetProgramInfoLog(mShaderProgram, 1024, NULL, infoLog);
            printf("%s \n",infoLog);
        }
        const GLint aPosLocation =  glGetAttribLocation(mShaderProgram, "aPos");
        const GLint aTexCoordLocation =  glGetAttribLocation(mShaderProgram, "aTexCoord");
        const float vertices[] = {
                -1.0f,-1.0f,0,0.0f,1.0f,
                1.0f,-1.0f,0,1.0f,1.0f,
                1.0f,1.0f,0,1.0f,0.0f,
                -1.0f,1.0f,0,0.0f,0.0f
        };
        const GLint indices[] = {
                0, 1, 3,
                1, 2, 3
        };

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &mEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(aPosLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(aPosLocation);
        glVertexAttribPointer(aTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(aTexCoordLocation);

        glGenTextures(1, &mTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

OpenGlWindow::~OpenGlWindow() {
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

void OpenGlWindow::draw(int width,int height,void *ptr) {
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,ptr);
    glUseProgram(mShaderProgram);
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glfwSwapBuffers(pWindow);
    glfwPollEvents();
}

