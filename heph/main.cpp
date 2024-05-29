#include "include/core/instance.hpp"
#include "src/core/instance.cpp"

int main()
{
        heph_instance_t h = {}; 
        heph_instance_init(&h);
        heph_instance_run(&h);
        heph_instance_shutdown(&h);
        return 0;
}







