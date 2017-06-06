#include "IObject.h"
#include <iostream>
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
struct A{

    static constexpr uint32_t getHash() { return ct::hashClassName(__PRETTY_FUNCTION__); }
    static constexpr size_t classIdx(){ return ct::classNameIdx(__PRETTY_FUNCTION__); }
    static const char* name(){return __PRETTY_FUNCTION__;}
};

struct B{
    static constexpr uint32_t getHash() { return ct::hashClassName(__PRETTY_FUNCTION__); }
    static constexpr size_t classIdx(){ return ct::classNameIdx(__PRETTY_FUNCTION__); }
    static const char* name(){return __PRETTY_FUNCTION__;}
};

int main(){
    std::cout << "A: " << A::getHash() << " " << A::classIdx() << " " << A::name() << std::endl;
    std::cout << "B: " << B::getHash() << " " << B::classIdx() << " " << B::name() << std::endl;
    return 0;
}
