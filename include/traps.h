#ifndef	_PROTECT_H_
#define	_PROTECT_H_

#define DP_BASE_HIGH_SHIFT 	24
#define DP_BASE_MID_SHIFT 	16
#define DP_LIMIT_SHIFT 		16

typedef struct descriptor		
{
	t_16	limit_low;		
	t_16	base_low;		
	t_8	base_mid;		
	t_8	attr1;			
	t_8	limit_high_attr2;	
	t_8	base_high;		
}DESCRIPTOR;


typedef struct s_gate
{
	t_16	offset_low;	
	t_16	selector;	
	t_8	dcount;		
	t_8	attr;
	t_16	offset_high;	
}GATE;

typedef struct s_tss {
	t_32	backlink;
	t_32	esp0;		
	t_32	ss0;		
	t_32	esp1;
	t_32	ss1;
	t_32	esp2;
	t_32	ss2;
	t_32	cr3;
	t_32	eip;
	t_32	flags;
	t_32	eax;
	t_32	ecx;
	t_32	edx;
	t_32	ebx;
	t_32	esp;
	t_32	ebp;
	t_32	esi;
	t_32	edi;
	t_32	es;
	t_32	cs;
	t_32	ss;
	t_32	ds;
	t_32	fs;
	t_32	gs;
	t_32	ldt;
	t_16	trap;
	t_16	iobase;	
	
}TSS;


#define	INDEX_DUMMY		0
#define	INDEX_FLAT_C		1
#define	INDEX_FLAT_RW		2
#define	INDEX_VIDEO		3
#define	INDEX_TSS		4
#define	INDEX_LDT_FIRST		5

#define	SELECTOR_DUMMY		   0	
#define	SELECTOR_FLAT_C		0x08
#define	SELECTOR_FLAT_RW	0x10
#define	SELECTOR_VIDEO		(0x18+3)
#define	SELECTOR_TSS		0x20	
#define SELECTOR_LDT_FIRST	0x28   //开始存放用户进程的LDT选择子

#define	SELECTOR_KERNEL_CS	SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW
#define	SELECTOR_KERNEL_GS	SELECTOR_VIDEO

#define LDT_SIZE		2

#define	DA_32			0x4000	
#define	DA_LIMIT_4K		0x8000	
#define	DA_DPL0			0x00	
#define	DA_DPL1			0x20	
#define	DA_DPL2			0x40	
#define	DA_DPL3			0x60	
#define	DA_DR			0x90	
#define	DA_DRW			0x92	
#define	DA_DRWA			0x93	
#define	DA_C			0x98	
#define	DA_CR			0x9A	
#define	DA_CCO			0x9C	
#define	DA_CCOR			0x9E	
#define	DA_LDT			0x82	
#define	DA_TaskGate		0x85	
#define	DA_386TSS		0x89	
#define	DA_386CGate		0x8C	
#define	DA_386IGate		0x8E	
#define	DA_386TGate		0x8F	

#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

#define	INT_VECTOR_IRQ0			0x20
#define	INT_VECTOR_IRQ8			0x28
//syscall number
#define	INT_VECTOR_SYS_CALL		0x80


//index of LDT
#define INDEX_LDT_C     0
#define INDEX_LDT_D    	1

#define vir2phys(seg_base, vir)	(t_32)(((t_32)seg_base) + (t_32)(vir))

#define LIMIT_4K_SHIFT 	12

#endif 
