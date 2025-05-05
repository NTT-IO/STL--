#include <iostream>
#include "shared_memory.h"
using namespace std;
class A
{
    int var;
    string name;
    double vt;
};
int main()
{
    shared_ptr<A> ptr = shared_memory<A>::make_shared_with_pool();
    return 0;
}