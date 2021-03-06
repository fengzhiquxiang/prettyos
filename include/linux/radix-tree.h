/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _RADIX_TREE_H_
#define _RADIX_TREE_H_

#define RADIX_TREE_SLOT_RESERVED ((void *)~0UL)

#define RADIX_TREE_MAP_SHIFT  4
#define RADIX_TREE_MAP_SIZE  (1U << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK  (RADIX_TREE_MAP_SIZE-1)
#define RADIX_TREE_INDEX_BITS  (8 * sizeof(unsigned long))

struct radix_tree_node {
	unsigned int	count;
	void		*slots[RADIX_TREE_MAP_SIZE];
};

struct radix_tree_path {
	struct radix_tree_node *node, **slot;
};

struct radix_tree_root {
	unsigned int		height;
	int			gfp_mask;
	struct radix_tree_node	*rnode;
};

#define RADIX_TREE_INIT(mask)	{0, (mask), NULL}

#define RADIX_TREE(name, mask) \
	struct radix_tree_root name = RADIX_TREE_INIT(mask)

#define INIT_RADIX_TREE(root, mask)	\
do {					\
	(root)->height = 0;		\
	(root)->gfp_mask = (mask);	\
	(root)->rnode = NULL;		\
} while (0)

extern int radix_tree_reserve(struct radix_tree_root *, unsigned long, void ***);
extern int radix_tree_insert(struct radix_tree_root *, unsigned long, void *);
extern void *radix_tree_lookup(struct radix_tree_root *, unsigned long);
extern int radix_tree_delete(struct radix_tree_root *, unsigned long);

#endif /* _LINUX_RADIX_TREE_H */
