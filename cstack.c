#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

struct node
{
    struct node* prev;
    unsigned int size;
    char data[];
};

typedef struct node* cstack_t;

struct stack_entry
{
    int reserved;
    cstack_t stack;
};

typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

struct stack_entries_table g_table = {0u, NULL};

hstack_t stack_new(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (!g_table.entries[i].reserved)
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }
    unsigned int new_size = g_table.size + 1;
    stack_entry_t* new_entries =
        realloc(g_table.entries, new_size * sizeof(stack_entry_t));
    if (new_entries == NULL)
    {
        return -1;
    }
    g_table.entries = new_entries;
    g_table.entries[g_table.size].reserved = 1;
    g_table.entries[g_table.size].stack = NULL;
    hstack_t new_handle = (hstack_t)g_table.size;
    g_table.size = new_size;
    return new_handle;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        struct node* prev = current->prev;
        free(current);
        current = prev;
    }
    g_table.entries[hstack].stack = NULL;
    g_table.entries[hstack].reserved = 0;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved)
        {
            stack_free((hstack_t)i);
        }
    }
    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0)
    {
        return 0;
    }
    if ((unsigned int)hstack >= g_table.size)
    {
        return 0;
    }
    return g_table.entries[hstack].reserved;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    unsigned int size = 0;
    struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        ++size;
        current = current->prev;
    }
    return size;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_in == NULL || size == 0)
    {
        return;
    }
    struct node* new_node = malloc(sizeof(struct node) + size);
    if (new_node == NULL)
    {
        return;
    }
    new_node->prev = g_table.entries[hstack].stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);
    g_table.entries[hstack].stack = new_node;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_out == NULL || size == 0)
    {
        return 0;
    }
    struct node* top = g_table.entries[hstack].stack;
    if (top == NULL)
    {
        return 0;
    }
    if (size < top->size)
    {
        return 0;
    }
    unsigned int data_size = top->size;
    memcpy(data_out, top->data, data_size);
    g_table.entries[hstack].stack = top->prev;
    free(top);
    return data_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    struct node* top = g_table.entries[hstack].stack;
    if (top == NULL)
    {
        return 0;
    }
    return top->size;
}

hstack_t stack_get_all(void)
{
    cstack_t new_stack = NULL;
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved)
        {
            struct node* new_node =
                malloc(sizeof(struct node) + sizeof(hstack_t));
            if (new_node == NULL)
            {
                while (new_stack != NULL)
                {
                    struct node* prev = new_stack->prev;
                    free(new_stack);
                    new_stack = prev;
                }
                return -1;
            }
            hstack_t handle = (hstack_t)i;
            new_node->prev = new_stack;
            new_node->size = sizeof(hstack_t);
            memcpy(new_node->data, &handle, sizeof(hstack_t));
            new_stack = new_node;
        }
    }
    hstack_t result = stack_new();
    if (result == -1)
    {
        while (new_stack != NULL)
        {
            struct node* prev = new_stack->prev;
            free(new_stack);
            new_stack = prev;
        }
        return -1;
    }
    g_table.entries[result].stack = new_stack;
    return result;
}
