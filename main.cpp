
#include "cpp/h264/Base.h"
#include "cpp/h264/Derived.h"

int main() {
    Base *base=new Derived;
    base->operation();
    delete base;
    Derived* derived=new Derived;
    derived->operation();
    delete derived;

}