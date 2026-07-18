#pragma once
#include <vector>

class Object{
    public:
        Object* get_parent();
        std::vector<Object*> get_children();
    private:
        Object* parent;
        std::vector<Object*> children;

};