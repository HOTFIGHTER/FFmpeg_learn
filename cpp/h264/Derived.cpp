//
// Created by xmly on 2019/6/30.
//

#include "Derived.h"

Derived::Derived() {
   std::cout<<"Derived"<<endl;
}

void Derived::operation() {
    std::cout<<"Derived_operation"<<endl;
}

Derived::~Derived() {
    std::cout<<"~Derived"<<endl;
}
