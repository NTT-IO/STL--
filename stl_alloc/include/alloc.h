#include <stdlib.h>
class alloc
{
private:
    static void *oom_malloc(size_t n)
    {
        void *result;
        for (;;)
        {
            result = malloc(n);
            if (result)
                return (result);
        }
    }
    static void *oom_realloc(void *p, size_t n)
    {
        void (*my_malloc_handler)();
        void *result;

        for (;;)
        {
            result = realloc(p, n);
            if (result)
                return (result);
        }
    }

public:
    static void *allocate(size_t n)
    {
        void *result = malloc(n);
        if (0 == result)
            result = oom_malloc(n);
        return result;
    }
    static void deallocate(void *p, size_t)
    {
        free(p);
    }
    static void *reallocate(void *p, size_t, size_t new_sz)
    {
        void *result = realloc(p, new_sz);
        if (0 == result)
            result = oom_realloc(p, new_sz);
        return result;
    }
};
