#include "heph_instance.hpp"
#include "heph_instance.cpp"

int main()
{
        heph_instance_t h = {}; 
        heph_instance_init(&h);
        heph_instance_run(&h);
        heph_instance_shutdown(&h);
        return 0;
}







