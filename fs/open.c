#include "type.h"
#include "const.h"
#include "wait.h"
#include "sched.h"
#include "panic.h"
#include "stdlib.h"
#include "hd.h"
#include "mm.h"
#include "blk_drv.h"
#include "file.h"
#include "printf.h"

//创建一个新的文件/目录
struct m_inode * new_file(struct m_inode *dir,char *basename,int namelen, int mode, int flags)
{
    int ret;
    struct m_inode *inode = NULL;
    int inode_nr = 0;
    inode_nr = get_imap_bit(dir->i_dev);
    printk("new inode_nr = %d.",inode_nr);
    printk("create_file:inode_nr = %d\n",inode_nr);
    if(inode_nr <= 0)
    {
        printk("there is no free inode \n");
        return NULL;
    }
    inode = iget(dir->i_dev,inode_nr);
    if(!inode)
    {
        printk("there is no free inode 2 \n");
        return NULL;
    }

    if((ret = add_entry(dir,inode->i_num,basename)) < 0)
    {
        printk("add_entry error.ret = %d.",ret);
        return NULL;
    }

    inode->i_mode = mode;
    inode->i_flags = flags;
    inode->i_dirty = 1;

    if((write_inode(dir)) < 0)
    {
        printk("write error.");
        return NULL;
    }

    printk("create file successful \n");
    return inode;
}

static inline int get_empty_fd(int *fd)
{
    unsigned int i;
    /* search the unused fd */
    for(i = 1;i < NR_OPEN;i++)
    {       
        if(current->filp[i] == NULL)
        {
            *fd = i;
            break;
        }
    }

    if(i <= 0 || (unsigned int)i > NR_OPEN)
    {
        /*panic("filp is full (PID %d)\n",proc2pid(current)); */
        return -1;
    }

    return 0;
}

static inline struct file * get_empty_filp()
{
    struct file *fp = NULL;

    if(nr_file_count++ >= NR_OPEN)
        return NULL;

    fp = (struct file *)kmem_get_obj(file_cachep);

    return fp;
}

int open(char* filename,int mode,int flag)
{
    int ret,fd;
    struct file *fp;
    struct m_inode *inode;

    if(filename == NULL)
        return -1;

    if(get_empty_fd(&fd) < 0)
        return -2;

    if((fp = get_empty_filp()) == NULL)
        return -3;

    //	printk("root_inode->i_dev = %d\n",root_inode->i_dev);
    if((ret = open_namei(filename,mode,flag,&inode)) < 0)
    {
        current->filp[fd] = NULL;
        fp->f_count = 0;
        return -4;
    }
    printk("inode->num = %d.inode->i_size = %d.",inode->i_num, inode->i_size);
    //返回文件句柄
    //	printk("open inode->i_num = %d\n",inode->i_num);
    struct super_block *sb = get_super_block(inode->i_dev);
    fp->f_inode = inode;
    fp->f_count = 1;
    fp->f_flag = flag;
    fp->f_mode = mode;
    fp->f_pos = 0;
    fp->f_op = &general_fop;
    fp->f_count++;

    current->filp[fd] = fp;
    list_add(&fp->f_list, &sb->s_files);

    printk("open:fd =  %d\n",fd);
    return fd;
}

/* make directory */
int mkdir(char* filename,int mode,int flag)
{
    int ret,fd;
    struct file *fp;
    struct m_inode *inode;

    if(filename == NULL)
        return -1;

    if(get_empty_fd(&fd) < 0)
        return -2;

    fp = get_empty_filp();
    if(fp == NULL)
        return -3;

    (current->filp[fd] = fp)->f_count++;
    //	printk("root_inode->i_dev = %d\n",root_inode->i_dev);
    if((ret = open_namei(filename,mode,flag,&inode)) < 0)
    {
        current->filp[fd] = NULL;
        fp->f_count = 0;
        return ret;
    }
    //返回文件句柄
    //	printk("open inode->i_num = %d\n",inode->i_num);
    fp->f_inode = inode;
    fp->f_count = 1;
    fp->f_flag = flag;
    fp->f_mode = mode;
    fp->f_pos = 0;
    fp->f_op = &general_fop;

    printk("open:fd =  %d\n",fd);
    return fd;
    return 0;
}

int close(int fd)
{
    struct file *fp = NULL;
    if(fd <= 0 || fd >= NR_OPEN)
        return -1;
    if(!(fp = current->filp[fd]))
        return -2;

    if(fp->f_count == 0)
        panic("Close:file count is 0");

    /*printk("4 count=%d", filp->f_count);*/
    int ret = iput(fp->f_inode);
    printk("iput ret = %d.",ret);
    if(--fp->f_count)
        return 0;
    current->filp[fd] = NULL;

    return 0;
}

int sys_close(int fd)
{
    return close(fd);
}
