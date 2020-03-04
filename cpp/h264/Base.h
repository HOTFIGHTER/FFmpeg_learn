//
// Created by xmly on 2019/6/30.
//

#ifndef FFMPEG_LEARN_BASE_H
#define FFMPEG_LEARN_BASE_H

#include <iostream>
using namespace std;
class Base {
public:
   Base();
   //virtual ~Base(); 析构设置成virtual才会析构基类
    ~Base();
   virtual void operation();
};


#endif //FFMPEG_LEARN_BASE_H
