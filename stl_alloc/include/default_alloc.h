#include "simple_alloc.h"
class default_alloc
{
    static char *start;
    static char *end;
    static size_t total_size;
    union block
    {
        char start[1];
        block *next;
    };
    static block *free_list[16];
    static size_t round_up(size_t bytes)
    {
        return (((bytes) + 8 - 1) & ~(8 - 1));
    }
    static size_t to_freelist_index(size_t bytes)
    {
        return (((bytes) + 8 - 1) / 8 - 1);
    }

public:
    static void *allocate(size_t n)
    {
        block **my_free_list;
        block *result;
        if (n > (size_t)128)
        {
            return (simple_alloc<int>::allocate(n));
        }
        my_free_list = free_list + to_freelist_index(n);
        result = *my_free_list;
        if (result == 0)
        {
            void *r = refill(round_up(n));
            return r;
        }
        *my_free_list = result->next;
        return (result);
    }
    static void *refill(size_t n)
    {
        int n_size = 20;
        char *chunk = chunk_alloc(n, n_size);
        block **my_free_list;
        block *result;
        block *current_obj, *next_obj;
        int i;
        if (1 == n_size)
            return chunk;
        my_free_list = free_list + to_freelist_index(n);
        result = (block *)chunk;
        *my_free_list = next_obj = (block *)(chunk + n);
        for (i = 1;; i++)
        {
            current_obj = next_obj;
            next_obj = (block *)((char *)next_obj + n);
            if (n_size - 1 == i)
            {
                current_obj->next = 0;
                break;
            }
            else
            {
                current_obj->next = next_obj;
            }
        }
        return result;
    }

    static char *chunk_alloc(size_t size, int &n_size)
    {
        char *result;
        size_t total_bytes = size * n_size;
        size_t bytes_left = end - start;

        if (bytes_left >= total_bytes)
        {
            result = start;
            start += total_bytes;
            return result;
        }
        else if (bytes_left >= size)
        {
            n_size = bytes_left / size;
            total_bytes = size * n_size;
            result = start;
            start += total_bytes;
            return result;
        }
        else
        {
            size_t bytes_to_get = 2 * total_bytes + round_up(total_size >> 4);
            if (bytes_left > 0)
            {
                block **my_free_list = free_list + to_freelist_index(bytes_left);

                ((block *)start)->next = *my_free_list;
                *my_free_list = (block *)start;
            }
            start = (char *)malloc(bytes_to_get);
            if (0 == start)
            {
                int i;
                block **my_free_list, *p;
                for (i = size; i <= 128; i += 8)
                {
                    my_free_list = free_list + to_freelist_index(i);
                    p = *my_free_list;
                    if (0 != p)
                    {
                        *my_free_list = p->next;
                        start = (char *)p;
                        end = start + i;
                        return (chunk_alloc(size, n_size));
                    }
                }
                end = 0;
                start = (char *)simple_alloc<int>::allocate(bytes_to_get);
            }
            total_size += bytes_to_get;
            end = start + bytes_to_get;
            return chunk_alloc(size, n_size);
        }
    }
    static void deallocate(void *p, size_t n)
    {
        block *q = (block *)p;
        block **my_free_list;

        if (n > (size_t)128)
        {
            alloc::deallocate(p, n);
            return;
        }
        my_free_list = free_list + to_freelist_index(n);
        q->next = *my_free_list;
        *my_free_list = q;
    }
};
char *default_alloc::start;
char *default_alloc::end;
size_t default_alloc::total_size;
default_alloc::block *default_alloc::free_list[16];