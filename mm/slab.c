#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "printf.h"
#include "mm.h"
#include "page_alloc.h"
#include "math.h"
#include "stddef.h"
#include "fs.h"
#include "sched.h"

struct kmem_list initkmem[NUM_INIT_LISTS];
struct list_head cache_chain;

struct kmem_cache *vma_cachep = NULL;
struct kmem_cache *tsk_cachep = NULL;
struct kmem_cache *thread_union_cachep = NULL;
struct kmem_cache *inode_cachep = NULL;
struct kmem_cache *file_cachep = NULL;
/*struct kmem_cache *dentry_cachep = NULL;*/

unsigned int * slab_bufctl(struct slab *slabp)
{
    return (unsigned int *)(slabp + 1);
}

int print_slab_info(struct slab *slabp)
{
    if(slabp == NULL)
    {
        printk("****");
        return -1;
    }

    printk("s_mem = %x.", slabp->s_mem);
    printk("free = %d.", slabp->free);
    printk("inuse = %d.\n", slabp->inuse);

    /*int tmp = slabp->free;
    while(tmp != BUFCTL_END)
    {
        printk("next = %d.",tmp);
        tmp = slab_bufctl(slabp)[tmp];
    }*/

    return 0;
}

unsigned int obj_to_index(struct kmem_cache *cachep, struct slab *slabp, void *objp)
{
    return ((objp - slabp->s_mem)/cachep->obj_size);
}

void * index_to_obj(struct kmem_cache *cachep, struct slab *slabp, unsigned int free)
{
    return (slabp->s_mem + cachep->obj_size * free);
}

void * slab_get_obj(struct kmem_cache *cachep, struct slab *slabp)
{
    if(cachep == NULL || slabp == NULL)
        return NULL;

    struct list_head *item = NULL;

    if(slabp->free == BUFCTL_END)
    {
        printk("not free obj.");
        return NULL;
    }
    void *objp = index_to_obj(cachep, slabp, slabp->free);
    unsigned int next = slab_bufctl(slabp)[slabp->free];
    /*printk("next = %d",next);*/
    slabp->inuse++;
    slabp->free = next;

    /* move to the full list*/
    if(slabp->inuse == cachep->obj_num)
    {
        item = &(slabp->list);
        list_del(item);
        list_add(item, &(cachep->lists.full));
    }

    return objp;
}

int slab_free_obj(struct kmem_cache *cachep, struct slab *slabp, void *objp)
{
    if(cachep == NULL || slabp == NULL || objp == NULL)
        return -1;

    struct list_head *item = NULL;

    unsigned int objnr = obj_to_index(cachep, slabp, objp);
    if(objnr > cachep->obj_num)
        return -2;
    slab_bufctl(slabp)[objnr] = slabp->free;
    slabp->free = objnr;
    slabp->inuse--;

    /* move to the free list */
    if(slabp->inuse == 0)
    {
        item = &(slabp->list);
        list_del(item);
        list_add(item, &(cachep->lists.free));
    }

    return 0;
}

/* alloc slab pages default for 4 pages */
inline void * slab_alloc()
{
    unsigned long address = get_free_pages(DEFAULT_SLAB_PAGES);

    return (void *)address;
}

/* estimate the slab parameters */
inline int slab_estimate(const int obj_size, int *obj_num)
{
    *obj_num = ((power(DEFAULT_SLAB_PAGES) * PAGE_SIZE) - sizeof(struct slab))/obj_size;

    return 0;
}

/* init slab 
 * struct slab + obj_num * (unsigned int)*/
int slab_init(struct kmem_cache *cachep, struct slab *slabp, void *args)
{
    int i = 0;
    void *ptr = args;
    /*slabp->free = cachep->obj_num;*/
    slabp->free = 0;
    slabp->inuse = 0;
    INIT_LIST_HEAD(&(slabp->list));

    /*printk("obj num = %d.",cachep->obj_num);*/
    ptr += (cachep->obj_num + 1) * sizeof(unsigned int);

    slabp->s_mem = ptr;
    for(i = 0;i < cachep->obj_num; ++i)
    {
        slab_bufctl(slabp)[i] = i + 1;
    }
    slab_bufctl(slabp)[i - 1] =  BUFCTL_END;

    return 0;
}

struct slab * kmem_get_slab(struct kmem_cache *cachep)
{
    struct slab *slabp = NULL;
    struct kmem_list *lists = NULL;
    struct list_head *head = NULL, *item = NULL, *pos = NULL;

    if(cachep == NULL)
        return NULL;

    lists = &(cachep->lists);

    /* first find  the partial slab*/
    head = &(lists->partial);
    if(list_empty_careful(head))
    {
        head = &(lists->free);
        if(list_empty_careful(head))
        {
            return NULL;
        }
        else
        {
            if(list_get_del(head, &item) != 0)
                return NULL;
            list_add(item, &(cachep->lists.partial));
        }
    }

    head = &(lists->partial);
    if(list_get(head, &pos) != 0)
    {
        return NULL;
    }

    slabp = list_entry(pos, struct slab, list);
    return slabp;
}

/* add a slab to kmem_cache */
int kmem_add_slab(struct kmem_cache *cachep, struct slab *slabp)
{
    if(cachep == NULL || slabp == NULL)
        return -1;

    /*cachep->obj_num += slabp->free;*/
    cachep->nr_frees += cachep->obj_num;
    list_add_tail(&(slabp->list), &(cachep->lists.free));

    return 0;
}

int kmem_free_slab(struct kmem_cache *cachep, struct slab *slabp)
{
    if(cachep == NULL || slabp == NULL)
        return -1;

    list_del(&(slabp->list));

    return 0;
}


/* create a slab and add to kmem_cache */
int cache_grow(struct kmem_cache *cachep)
{
    if(cachep == NULL || cachep->obj_size == 0)
    {
        return -1;
    }
    void *ptr;
    struct slab *slabp = NULL;
    /*unsigned long slab_size;*/
    int obj_num = 0;

    ptr = slab_alloc();

    /*slab_size = sizeof(struct slab) + obj_size * obj_num;*/

    slabp = (struct slab *)ptr;
    slab_estimate(cachep->obj_size, &obj_num);
    cachep->obj_num = obj_num;
    cachep->nr_pages += power( DEFAULT_SLAB_PAGES);

    ptr += sizeof(struct slab);
    slab_init(cachep, slabp, ptr);

    kmem_add_slab(cachep, slabp);

    return 0;
}


void * kmem_get_obj(struct kmem_cache *cachep)
{
    struct slab *slabp = NULL;
    void *obj = NULL;

repeat:
    slabp = kmem_get_slab(cachep);
    obj = slab_get_obj(cachep, slabp);

    if(obj == NULL)
    {
        if(cachep->nr_pages >= power(MAX_SLAB_PAGES))
        {
            return NULL;
        }
        else
        {
            cache_grow(cachep);
            goto repeat;
        }
    }

    return obj;
}

int kmem_free_obj(struct kmem_cache *cachep, void *objp)
{
    if(cachep == NULL || objp == NULL)
        return -1;

    /*unsigned long address = ((unsigned long )objp & (~(DEFAULT_SLAB_PAGES * PAGE_SIZE) + 1));*/
    unsigned long address = ((unsigned long )objp & VALUE_MASK(DEFAULT_SLAB_PAGES * PAGE_SIZE));
    struct slab *slabp = (struct slab *)address;

    return slab_free_obj(cachep,slabp,objp);
}

int print_kmem_info(struct kmem_cache *cachep)
{
    if(cachep == NULL)
        return -1;

    printk("name = %s.", cachep->name);
    printk("obj_size = %d.", cachep->obj_size);
    /*printk("obj_num = %d.", cachep->objobj__num);*/
    printk("partial num = %d.",list_num(&(cachep->lists.partial)));
    printk("full num = %d.",list_num(&(cachep->lists.full)));
    printk("free num = %d.",list_num(&(cachep->lists.free)));
    printk("nr_frees = %d.\n", cachep->nr_frees);

    return 0;
}

void print_kmem_chain()
{
    struct kmem_cache *cachep = NULL;
    struct list_head *head, *pos, *n;

    head = &(cache_chain);

    list_for_each_safe(pos, n, head)
    {
        cachep = list_entry(pos, struct kmem_cache, next);
        print_kmem_info(cachep);
    }
}

/* init kmem_list  */
int kmem_list_init(struct kmem_list *parent)
{
    INIT_LIST_HEAD(&(parent->partial));
    INIT_LIST_HEAD(&(parent->full));
    INIT_LIST_HEAD(&(parent->free));
    /*parent->free_obj = 0;*/

    return 0;
}

/* the slab system start  */
int kmem_cache_init()
{
    INIT_LIST_HEAD(&cache_chain);

    return 0;
}

int kmem_chain_add(struct kmem_cache *cachep)
{
    if(cachep == NULL)
        return -1;

    struct list_head *head = NULL, *pos, *n;
    struct kmem_cache *item_cachep = NULL;

    head = &(cache_chain);

    if(list_empty_careful(head))
    {
        list_add(&(cachep->next),&(cache_chain));
    }
    else
    {
        list_for_each_safe(pos, n, head)
        {
            item_cachep = list_entry(pos, struct kmem_cache, next);
            if(cachep->obj_size < item_cachep->obj_size)
            {
                __list_add(&(cachep->next), pos->prev, pos);
                return 0;
            }
            if(pos->next == head)
            {
                __list_add(&(cachep->next), pos, n);
                return 0;
            }
        }
    }

    return -1;
}

/* alloc kmem_cache */
void * kmem_cache_alloc()
{
    /*get a free page from buddy list
     * kmem_cache struct must in one page*/
    if(sizeof(struct kmem_cache)> PAGE_SIZE)
        return NULL;

    int order = (sizeof(struct kmem_cache) >>PAGE_SHIFT);
    unsigned long address = get_free_pages(order);

    return (void *)address;
}

/* just only create kmem_cache 
 * and add to cache_chain*/
struct kmem_cache * kmem_cache_create(char *name, size_t obj_size, unsigned long flags)
{
    void *ptr;
    struct kmem_cache *cachep = NULL;
    /*unsigned long cache_size = 0;*/

    if(name == NULL || obj_size == 0)
        return NULL;

    ptr = kmem_cache_alloc();

    if(ptr == NULL)
        return NULL;

    cachep = (struct kmem_cache *)ptr;
    strncpy(cachep->name, name, sizeof(cachep->name));
    cachep->obj_size = obj_size;
    cachep->nr_frees = 0;
    cachep->obj_num = 0;
    cachep->flags = flags;
    cachep->nr_pages = 0;

    kmem_list_init(&(cachep->lists));
    /*cache_grow(cachep, obj_size);*/
    cache_grow(cachep);
    kmem_chain_add(cachep);

    return cachep;
    /*print_kmem_info(cachep);*/
}

/* get cache size */
unsigned int kmem_cache_size(struct kmem_cache *cachep)
{
    return cachep->obj_size;
}

/* get cache num */
unsigned int kmem_cache_num(struct kmem_cache *cachep)
{
    return cachep->obj_num;
}

/* get cahce name */
const char * kmem_cache_name(struct kmem_cache *cachep)
{
    return cachep->name;
}

/* get cache frees */
unsigned int kmem_cache_frees(struct kmem_cache *cachep)
{
    return cachep->nr_frees;
}

int init_kmem_cache()
{
    tsk_cachep = kmem_cache_create("tsk",sizeof(struct task_struct),0);
    if(tsk_cachep == NULL)
        return -1;

    thread_union_cachep =  kmem_cache_create("task_union",sizeof(union thread_union),0);
    if(thread_union_cachep == NULL)
        return -2;

    vma_cachep = kmem_cache_create("vma", sizeof(struct vm_area_struct),0);
    if(vma_cachep == NULL)
        return -3;

    inode_cachep = kmem_cache_create("inode", sizeof(struct m_inode), 0);
    if(inode_cachep == NULL)
        return -4;

    file_cachep = kmem_cache_create("file", sizeof(struct file), 0);
    if(file_cachep == NULL)
        return -5;

    return 0;
}
