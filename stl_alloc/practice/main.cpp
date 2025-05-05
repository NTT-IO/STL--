#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <iomanip>
using namespace std;
#define PCB_SIZE sizeof(PCB)
struct MemoryBlock
{
private:
    char *start;
    size_t length;
    struct Block
    {
    private:
        friend struct MemoryBlock;
        char *start;
        size_t length;
        bool is_empty;
        Block(char *start, size_t length, bool is_empty = false) : start(start), length(length), is_empty(is_empty) {}
    };
    list<Block *> blocks;

public:
    MemoryBlock(size_t length)
    {
        start = (char *)malloc(length);
        blocks.push_back(new Block(start, length, true));
    }

    ~MemoryBlock()
    {
        for (auto block : blocks)
            delete block;
        free(start);
    }
    void *alloc(size_t size)
    {
        Block *max = *blocks.begin();
        list<Block *>::iterator max_it = blocks.begin();
        for (list<Block *>::iterator it = blocks.begin(); it != blocks.end(); ++it)
        {
            Block *block = *it;
            if (block->is_empty && block->length >= size && block->length >= max->length)
            {
                max = block;
                max_it = it;
            }
        }
        if (max->is_empty)
        {
            char *ptr = max->start;
            if (max->length == size)
            {
                max->is_empty = false;
                return ptr;
            }
            else if (max->length > size)
            {
                blocks.insert(max_it, new Block(ptr, size));
                max->start += size;
                max->length -= size;
                return ptr;
            }
        }
        return nullptr;
    }
    template <typename T>
    void dealloc(T *ptr)
    {
        char *p = (char *)ptr;
        for (list<Block *>::iterator it = blocks.begin(); it != blocks.end(); ++it)
        {
            Block *block = *it;
            if (block->start == p)
            {
                if (!block->is_empty)
                {
                    if (it != blocks.begin())
                    {
                        list<Block *>::iterator prev_it = std::prev(it);
                        Block *prev_block = *prev_it;
                        if (prev_block->is_empty)
                        {
                            prev_block->length += block->length;
                            delete block;
                            blocks.erase(it);
                            it = prev_it;
                            block = prev_block;
                        }
                    }
                    list<Block *>::iterator next_it = std::next(it);
                    if (next_it != blocks.end())
                    {
                        Block *next_block = *next_it;
                        if (next_block->is_empty)
                        {
                            block->length += next_block->length;
                            delete next_block;
                            blocks.erase(next_it);
                        }
                    }
                    block->is_empty = true;
                }
                return;
            }
        }
    }
    void display(string name) const
    {
        cout << name << " Layout " << endl;
        cout << setw(18) << "起始地址" << setw(22) << "长度" << setw(20) << "状态" << endl;
        for (const auto &block : blocks)
        {
            cout << "| " << setw(18) << (void *)block->start << "| "
                 << setw(18) << block->length << "| "
                 << setw(18) << (block->is_empty ? "Free" : "Used") << "| " << endl;
        }
    }
};
struct PCB
{
private:
    static int ID;
    PCB(PCB *next, int size) : next(next), size(size)
    {
        string temp = "P_";
        temp += to_string(ID++);
        strcpy(name, temp.c_str());
    }
    ~PCB() {}

public:
    char name[8];
    PCB *next;
    char *start;
    int size;
    static PCB *create_PCB(MemoryBlock &PCB_MB, MemoryBlock &FUNC_MB, int size, PCB *next = nullptr)
    {
        PCB *pcb = (PCB *)PCB_MB.alloc(PCB_SIZE);
        if (!pcb)
            return nullptr;
        new (pcb) PCB(next, size);
        pcb->start = (char *)FUNC_MB.alloc(size);
        return pcb;
    }
};
int PCB::ID = 0;

struct ready_queue
{
    PCB *pcb;
    ready_queue *next;
};
ready_queue *rq_head;

struct clog_queue
{
    PCB *pcb;
    clog_queue *next;
};

clog_queue *cq_head;

PCB *PCB_head;

void process_control(MemoryBlock &PCB_mb, MemoryBlock &FUNC_mb)
{
    int choice;
    while (true)
    {
        cout << "\n===== 进程控制菜单 =====" << endl;
        cout << "1. 创建新进程" << endl;
        cout << "2. 时间片到" << endl;
        cout << "3. 阻塞执行进程" << endl;
        cout << "4. 唤醒第一个阻塞进程" << endl;
        cout << "5. 终止执行进程" << endl;
        cout << "0. 退出" << endl;
        cout << endl;
        cout << "运行进程: " << (PCB_head ? PCB_head->name : "无") << endl;
        cout << "就绪队列: ";
        ready_queue *rq = rq_head;
        while (rq != nullptr)
        {
            cout << rq->pcb->name << " ";
            rq = rq->next;
        }
        cout << endl;
        cout << "阻塞队列: ";
        clog_queue *cq = cq_head;
        while (cq != nullptr)
        {
            cout << cq->pcb->name << " ";
            cq = cq->next;
        }
        cout << endl;
        PCB_mb.display("PCB");
        FUNC_mb.display("FUNC");
        cout << "请输入选择: ";
        cin >> choice;
        switch (choice)
        {
        case 1:
        {
            size_t size;
            cin >> size;
            PCB *newPCB = PCB::create_PCB(PCB_mb, FUNC_mb, size);
            if (rq_head == nullptr)
            {
                rq_head = new ready_queue();
                rq_head->pcb = newPCB;
                rq_head->next = nullptr;
            }
            else
            {
                ready_queue *temp = rq_head;
                while (temp->next != nullptr)
                {
                    temp = temp->next;
                }
                temp->next = new ready_queue();
                temp->next->pcb = newPCB;
                temp->next->next = nullptr;
            }
            if (PCB_head == nullptr && rq_head != nullptr)
            {
                PCB_head = rq_head->pcb;
                ready_queue *toDelete = rq_head;
                rq_head = rq_head->next;
                delete toDelete;
            }
            break;
        }
        case 2:
        {
            if (PCB_head != nullptr)
            {
                ready_queue *temp = rq_head;
                if (temp == nullptr)
                {
                    rq_head = new ready_queue();
                    rq_head->pcb = PCB_head;
                    rq_head->next = nullptr;
                }
                else
                {
                    while (temp->next != nullptr)
                    {
                        temp = temp->next;
                    }
                    temp->next = new ready_queue();
                    temp->next->pcb = PCB_head;
                    temp->next->next = nullptr;
                }
                PCB_head = nullptr;
            }
            if (rq_head != nullptr)
            {
                PCB_head = rq_head->pcb;
                ready_queue *toDelete = rq_head;
                rq_head = rq_head->next;
                delete toDelete;
            }
            if (PCB_head == nullptr && rq_head != nullptr)
            {
                PCB_head = rq_head->pcb;
                ready_queue *toDelete = rq_head;
                rq_head = rq_head->next;
                delete toDelete;
            }
            break;
        }
        case 3:
        {
            if (PCB_head != nullptr)
            {
                clog_queue *newBlocked = new clog_queue();
                newBlocked->pcb = PCB_head;
                newBlocked->next = nullptr;
                if (cq_head == nullptr)
                    cq_head = newBlocked;
                else
                {
                    clog_queue *temp = cq_head;
                    while (temp->next != nullptr)
                    {
                        temp = temp->next;
                    }
                    temp->next = newBlocked;
                }
                PCB_head = nullptr;
                if (PCB_head == nullptr && rq_head != nullptr)
                {
                    PCB_head = rq_head->pcb;
                    ready_queue *toDelete = rq_head;
                    rq_head = rq_head->next;
                    delete toDelete;
                }
            }
            break;
        }
        case 4:
        {
            if (cq_head != nullptr)
            {
                PCB *unblocked = cq_head->pcb;
                clog_queue *toDelete = cq_head;
                cq_head = cq_head->next;
                delete toDelete;
                if (rq_head == nullptr)
                {
                    rq_head = new ready_queue();
                    rq_head->pcb = unblocked;
                    rq_head->next = nullptr;
                }
                else
                {
                    ready_queue *temp = rq_head;
                    while (temp->next != nullptr)
                    {
                        temp = temp->next;
                    }
                    temp->next = new ready_queue();
                    temp->next->pcb = unblocked;
                    temp->next->next = nullptr;
                }
                if (PCB_head == nullptr && rq_head != nullptr)
                {
                    PCB_head = rq_head->pcb;
                    ready_queue *toDelete = rq_head;
                    rq_head = rq_head->next;
                    delete toDelete;
                }
            }
            break;
        }
        case 5:
        {
            if (PCB_head != nullptr)
            {
                PCB *pcb = PCB_head;
                FUNC_mb.dealloc(pcb->start);
                PCB_mb.dealloc(pcb);
                if (rq_head != nullptr)
                {
                    PCB_head = rq_head->pcb;
                    ready_queue *temp = rq_head;
                    rq_head = rq_head->next;
                    delete temp;
                }
                else
                    PCB_head = nullptr;
            }
            break;
        }
        case 0:
            return;
        default:
            break;
        }
        cout << endl;
        system("cls");
    }
}
int main()
{
    MemoryBlock PCB_mb(150);
    MemoryBlock FUNC_mb(400);
    process_control(PCB_mb, FUNC_mb);
    return 0;
}