//
// Created by xmly on 2020/2/8.
//

#ifndef FFMPEG_LEARN_OPENGLWINDOW_H
#define FFMPEG_LEARN_OPENGLWINDOW_H
#include <glad.h>
#include <glfw3.h>
#include <printf.h>
class OpenGlWindow {
private:
    GLuint mTexture;
    GLuint mShaderProgram;
    GLuint mVAO;
    GLuint mVBO;
    GLuint mEBO;
    GLFWwindow *pWindow;
public:
    OpenGlWindow();
    ~OpenGlWindow();
    void draw(int width,int height,void *ptr);
};


#endif //FFMPEG_LEARN_OPENGLWINDOW_H
