#include "object.h"

std::vector<Object*> Object::get_children(){
    return children;
}

Object* Object::get_parent(){
    return  parent;
}