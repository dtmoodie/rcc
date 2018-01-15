#include "RuntimeObjectSystem/IObject.h"
#include "ct/Object.hpp"
#include <iostream>
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
struct Alpha{
    DECLARE_CLASS_HASH
    
    static constexpr size_t classIdx(){ return ct::classNameIdx(__PRETTY_FUNCTION__); }
    static const char* name(){return __PRETTY_FUNCTION__;}
};

struct Bravo{
    DECLARE_CLASS_HASH
    
    static constexpr size_t classIdx(){ return ct::classNameIdx(__PRETTY_FUNCTION__); }
    static const char* name(){return __PRETTY_FUNCTION__;}
};

int main(){
    static_assert(Alpha::getHash() != Bravo::getHash(), "A::getHash() != B::getHash()");
    std::cout << "A: " << Alpha::getHash() << " " << Alpha::classIdx() << " " << Alpha::name() << std::endl;
    std::cout << "B: " << Bravo::getHash() << " " << Bravo::classIdx() << " " << Bravo::name() << std::endl;
    return 0;
}
