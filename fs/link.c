#include "type.h"
#include "const.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "errno.h"
#include "printf.h"

int do_unlink(char *name)
{
    char *basename;
    int namelen;
    struct m_inode *dir,*inode;
    //	struct buffer_head *bh;
    struct dentry *de;
    if(dir_namei(name,&basename,&namelen, &dir) < 0)
    {	//no such file or direntry
        return -ENOENT;
    }
    if(!namelen)
    {
        iput(dir);
        return -ENOENT;
    }
    //删除目录项
    if((del_entry(dir,basename,namelen,&de)) != 0)
    {
        printk("no such file");
        iput(dir);
        return -ENOENT;		
    }
    if(!(inode = iget(dir->i_dev,de->inode_num)))
    {
        iput(dir);
        return -ENOENT;
    }
    //not permit to del direntry
    if(S_ISDIR(inode->i_mode))
    {
        iput(inode);
        iput(dir);
        return -EPERM;
    }
    if(--(inode->i_count))
    {
        printk("deleting using file\n");
        return -EMLINK;
    }
    de->inode_num = 0;
    inode->i_dirty = 1;
    iput(inode);
    iput(dir);
    printk("delete file successful\n");	
    return 0;
}
