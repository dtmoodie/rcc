#include "RuntimeObjectSystem/IObjectInfo.h"

IObjectInfo::~IObjectInfo(){

}

bool IObjectInfo::InheritsFrom(unsigned int id) const{
    (void)id;
    return false;
}
