//
// Created by xmly on 2019/6/30.
//

#ifndef FFMPEG_LEARN_DERIVED_H
#define FFMPEG_LEARN_DERIVED_H


#include "Base.h"

class Derived : public Base{
public:
    Derived();
    ~Derived();
    virtual void operation();
};


#endif //FFMPEG_LEARN_DERIVED_H
