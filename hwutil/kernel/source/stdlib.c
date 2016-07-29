/*
 * File         : stdlib.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "list.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "pthread.h"
#include "shell.h"

/*@{*/

#define MALLOC_MIN_SIZE                 12

#define MALLOC_DEBUG                    0

/* malloc debug function definition */
#if 0
    #define MALLOC_DEBUG( x ) printk x
#else
    #define MALLOC_DEBUG( x )
#endif

/*@}*/

/* the heap memory structure description */
struct heap_mem
{
   list_t       list;
  
   bool         used;
};
typedef struct heap_mem heap_mem_t;

/*@{*/

/* global heap mutex */
NO_INIT static pthread_mutex_t heap_mem_mutex;

NO_INIT static size_t mem_total_size;
NO_INIT static size_t mem_used_size;

NO_INIT static heap_mem_t *heap_mem_begin;
NO_INIT static heap_mem_t *heap_mem_end;
 
NO_INIT static heap_mem_t *heap_mem_free;
/*@}*/

/*@{*/

#if MALLOC_DEBUG
static void heap_mem_test(void)
{
    char *p[320];
    int i = 0;
  
    while (i < 10)
    {
        p[i++] = malloc(32);
    }
    
    i = 0;
    while (i < 10)
    {
        free(p[i++]);
    }
    
    while(1);
}
#endif

/*
 * the function will allocate a block of memory
 *
 * @param begin_addr the address of begin of the memory
 * @param end_addr   the address of end of the memory
 */
void heap_mem_init(phys_addr_t begin_addr, phys_addr_t end_addr)
{
    phys_addr_t align_begin_addr = ALIGN(begin_addr);
    phys_addr_t align_end_addr = ALIGN(end_addr);
   
    if ((align_end_addr - align_begin_addr) <= (2 * sizeof(heap_mem_t) + MALLOC_MIN_SIZE))
        return ;
 
    mem_total_size = align_end_addr - align_begin_addr - 2 * sizeof(heap_mem_t);
    heap_mem_begin = (heap_mem_t *)align_begin_addr;
    heap_mem_end   = (heap_mem_t *)(align_end_addr - sizeof(heap_mem_t));
    
    MALLOC_DEBUG(("Init heap memory addr from 0x%8x to 0x%8x. total is %d=%dk.\r\n",
    		heap_mem_begin,
    		heap_mem_end,
    		mem_total_size, mem_total_size / 1024));
    
    list_init(&heap_mem_begin->list);
    heap_mem_begin->used = false;
    
    list_init(&heap_mem_end->list);
    heap_mem_end->used = true;
        
    list_insert_head(&heap_mem_begin->list, &heap_mem_end->list);
    
    pthread_mutex_init(&heap_mem_mutex, NULL);
    
    heap_mem_free = heap_mem_begin;
    
    mem_used_size = 0;
    
#if MALLOC_DEBUG    
    heap_mem_test();
#endif
}

/*
 * the function will alloc a block of memory
 *
 * @param size the memory size
 *
 * @return point of the memory base address
 */
void *malloc(size_t size)
{   
    heap_mem_t *mem_ptr;
  
    if (!size)
        return NULL;
    
    /* there is min memory alloc block */
    if (size < MALLOC_MIN_SIZE)
        size = MALLOC_MIN_SIZE;
    
    /* memory must align */
    size = ALIGN(size);
    
    if (size > mem_total_size - mem_used_size)
        return NULL;
    
    pthread_mutex_lock(&heap_mem_mutex);
    
    LIST_FOR_EACH_ENTRY_FROM_PTR(mem_ptr,
    		heap_mem_free, heap_mem_end, heap_mem_t, list)
    {
        size_t mem_len = (size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) -
        		(size_t)mem_ptr - sizeof(heap_mem_t);
        
        if ((false == mem_ptr->used) && mem_len > size)
        {
            if ((mem_len - size) > (sizeof(heap_mem_t) + MALLOC_MIN_SIZE))
            {
                heap_mem_t *free_mem_ptr = (heap_mem_t *)((size_t)mem_ptr + size + sizeof(heap_mem_t));
                
                free_mem_ptr->used = false;               
                list_init(&free_mem_ptr->list);
                
                list_insert_head(&mem_ptr->list, &free_mem_ptr->list);
            }

            mem_ptr->used = true;
            
            mem_used_size += (size + sizeof(heap_mem_t));
            
            MALLOC_DEBUG(("malloc addr is 0x%8x, real addr start 0x%8x - 0x%8x.",
            		(size_t)mem_ptr + sizeof(heap_mem_t),
            		(size_t)mem_ptr,
            		(size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) - 1));
            MALLOC_DEBUG(("alloc memory is %d, totally use memory is %d.\r\n",
            		(size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) - (size_t)mem_ptr,
            		mem_used_size));
            
            if (heap_mem_free == mem_ptr)
            {
                while ((heap_mem_free->used == true) && (heap_mem_free != heap_mem_end))
                {
                    heap_mem_free = LIST_HEAD_ENTRY(&heap_mem_free->list, heap_mem_t, list);
                }
                ASSERT_KERNEL(!(heap_mem_free->used == true || heap_mem_free == heap_mem_end));                
            }
            
            pthread_mutex_unlock(&heap_mem_mutex);
            
            return (char *)mem_ptr + sizeof(heap_mem_t);
        }
    }
    
    pthread_mutex_unlock(&heap_mem_mutex);
    
    return NULL;
}

/*
 * calloc - the function will alloc a block of memory and clear it 
 *
 * @param size the memory size
 *
 * @return point of the memory base address
 */
void *calloc(size_t size)
{
    char *p = malloc(size);
      
    if (p)
        memset(p, 0, size );
  
    return p;
}


/*
 * free - the function will free the allocated memory
 *
 * @param mem the point of the memory
 */
void free(void *mem)
{
    heap_mem_t *mem_ptr, *__free_mem;
    heap_mem_t *prev_mem_ptr, *next_mem_ptr;
    size_t size;
  
    /* make sure the memory is meaning */
    if (NULL == mem)
        return ;
    
    /* make sure the memory is at the block of */
    if ((size_t)mem <= (size_t)heap_mem_begin || (size_t)mem >= (size_t)heap_mem_end)
        return ;
    
    /* make sure the memory is align */
    ASSERT_KERNEL((size_t)mem == ALIGN((size_t)mem));
    
    mem_ptr = (heap_mem_t *)((size_t)mem - sizeof(heap_mem_t));
       
    ASSERT_KERNEL(mem_ptr->used == true);
    
    /* lock the memory */
    pthread_mutex_lock(&heap_mem_mutex);

    /* free the memory block */
    mem_ptr->used = false;
    
    size = (size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) - (size_t)mem_ptr;
    
    /* if the next block memory is free, remove the next memory block */
    next_mem_ptr = LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list);
    if (false == next_mem_ptr->used)
    {
        list_remove_node(&next_mem_ptr->list);
        next_mem_ptr = LIST_HEAD_ENTRY(&next_mem_ptr->list, heap_mem_t, list);
    }
    
    /* if the previous block is free, delete the current block */
    prev_mem_ptr = LIST_TAIL_ENTRY(&mem_ptr->list, heap_mem_t, list);
    if (false == prev_mem_ptr->used)
    {
        list_remove_node(&mem_ptr->list);
        
        /* free block set the point to previous block */
        __free_mem = prev_mem_ptr;
    }
    else
        /* free block set the point to current block */
        __free_mem = mem_ptr;
    
    if (__free_mem < heap_mem_free)
        heap_mem_free = __free_mem;
          
    mem_used_size -= size;
    
    MALLOC_DEBUG(("free addr is 0x%8x, real addr start 0x%8x - 0x%8x.",
    		(size_t)mem_ptr + sizeof(heap_mem_t),
    		(size_t)mem_ptr,
    		(size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) - 1));
    MALLOC_DEBUG(("free memory is %d, totally use memory is %d.",
    		(size_t)LIST_HEAD_ENTRY(&mem_ptr->list, heap_mem_t, list) - (size_t)mem_ptr,
    		mem_used_size));
    MALLOC_DEBUG(("memory address is 0x%8x - 0x%8x. free is 0x%8x.\r\n",
    		(size_t)prev_mem_ptr,
    		(size_t)next_mem_ptr,
    		(size_t)heap_mem_free));
    
    pthread_mutex_unlock(&heap_mem_mutex);
}

static void ifmem(struct shell_dev *shell_dev)
{
    size_t tmem = (size_t)heap_mem_end - (size_t)heap_mem_begin;
  
    shell_printk(shell_dev, "\r\nmemory(B) %-10s%-10s%-10s%-10s", "type", 
                                                                  "total", 
                                                                  "used",
                                                                  "free");

    shell_printk(shell_dev, "\r\n          %-10s%-10d%-10d%-10d", "heap",
                                                                   tmem, 
                                                                   mem_used_size, 
                                                                   tmem - mem_used_size);  
}
SHELL_CMD_EXPORT(ifmem, show target system memory usage, 1);
