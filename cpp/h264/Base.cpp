//
// Created by xmly on 2019/6/30.
//

#include "Base.h"

Base::Base() {
   std::cout<<"Base"<<endl;
}

void Base::operation() {
   std::cout<<"Base_operation"<<endl;
}

Base::~Base() {
    std::cout<<"~Base"<<endl;
}
