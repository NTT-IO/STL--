#include <memory>
#include "default_alloc.h"
template <typename T>
class adapt_alloc
{
public:
    adapt_alloc(int size) : _size(size) {}
    void operator()(T *p)
    {
        default_alloc::deallocate(p, _size);
    }

private:
    int _size;
};

template <typename T>
class shared_memory
{

public:
    static std::shared_ptr<T> make_shared_with_pool()
    {
        std::shared_ptr<T> ptr(static_cast<T *>(default_alloc::allocate(sizeof(T))),
                               adapt_alloc<T>(sizeof(T)));
        return ptr;
    }
};