/*
 *  Copyright (C) 2002-2010  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* $Id: cpu.h,v 1.57 2009-05-27 09:15:40 qbix79 Exp $ */

#ifndef DOSBOX_CPU_H
#define DOSBOX_CPU_H

#ifndef DOSBOX_DOSBOX_H
#include "dosbox.h" 
#endif
#ifndef DOSBOX_REGS_H
#include "regs.h"
#endif
#ifndef DOSBOX_MEM_H
#include "mem.h"
#endif

#define CPU_AUTODETERMINE_NONE		0x00
#define CPU_AUTODETERMINE_CORE		0x01
#define CPU_AUTODETERMINE_CYCLES	0x02

#define CPU_AUTODETERMINE_SHIFT		0x02
#define CPU_AUTODETERMINE_MASK		0x03

#define CPU_CYCLES_LOWER_LIMIT		100


#define CPU_ARCHTYPE_MIXED			0xff
#define CPU_ARCHTYPE_386SLOW		0x30
#define CPU_ARCHTYPE_386FAST		0x35
#define CPU_ARCHTYPE_486OLDSLOW		0x40
#define CPU_ARCHTYPE_486NEWSLOW		0x45
#define CPU_ARCHTYPE_PENTIUMSLOW	0x50

/* CPU Cycle Timing */
extern System::Int32 CPU_Cycles;
extern System::Int32 CPU_CycleLeft;
extern System::Int32 CPU_CycleMax;
extern System::Int32 CPU_OldCycleMax;
extern System::Int32 CPU_CyclePercUsed;
extern System::Int32 CPU_CycleLimit;
extern System::Int64 CPU_IODelayRemoved;
extern bool CPU_CycleAutoAdjust;
extern bool CPU_SkipCycleAutoAdjust;
extern unsigned CPU_AutoDetermineMode;

extern unsigned CPU_ArchitectureType;

extern unsigned CPU_PrefetchQueueSize;

/* Some common Defines */
/* A CPU Handler */
typedef int (CPU_Decoder)(void);
extern CPU_Decoder * cpudecoder;

int CPU_Core_Normal_Run(void);
int CPU_Core_Normal_Trap_Run(void);
int CPU_Core_Simple_Run(void);
int CPU_Core_Full_Run(void);
int CPU_Core_Dyn_X86_Run(void);
int CPU_Core_Dyn_X86_Trap_Run(void);
int CPU_Core_Dynrec_Run(void);
int CPU_Core_Dynrec_Trap_Run(void);
int CPU_Core_Prefetch_Run(void);
int CPU_Core_Prefetch_Trap_Run(void);

void CPU_Enable_SkipAutoAdjust(void);
void CPU_Disable_SkipAutoAdjust(void);
void CPU_Reset_AutoAdjust(void);


//CPU Stuff

extern System::UInt16 parity_lookup[256];

bool CPU_LLDT(unsigned selector);
bool CPU_LTR(unsigned selector);
void CPU_LIDT(unsigned limit,unsigned base);
void CPU_LGDT(unsigned limit,unsigned base);

unsigned CPU_STR(void);
unsigned CPU_SLDT(void);
unsigned CPU_SIDT_base(void);
unsigned CPU_SIDT_limit(void);
unsigned CPU_SGDT_base(void);
unsigned CPU_SGDT_limit(void);

void CPU_ARPL(unsigned & dest_sel,unsigned src_sel);
void CPU_LAR(unsigned selector,unsigned & ar);
void CPU_LSL(unsigned selector,unsigned & limit);

void CPU_SET_CRX(unsigned cr,unsigned value);
bool CPU_WRITE_CRX(unsigned cr,unsigned value);
unsigned CPU_GET_CRX(unsigned cr);
bool CPU_READ_CRX(unsigned cr,System::UInt32 & retvalue);

bool CPU_WRITE_DRX(unsigned dr,unsigned value);
bool CPU_READ_DRX(unsigned dr,System::UInt32 & retvalue);

bool CPU_WRITE_TRX(unsigned dr,unsigned value);
bool CPU_READ_TRX(unsigned dr,System::UInt32 & retvalue);

unsigned CPU_SMSW(void);
bool CPU_LMSW(unsigned word);

void CPU_VERR(unsigned selector);
void CPU_VERW(unsigned selector);

void CPU_JMP(bool use32,unsigned selector,unsigned offset,unsigned oldeip);
void CPU_CALL(bool use32,unsigned selector,unsigned offset,unsigned oldeip);
void CPU_RET(bool use32,unsigned bytes,unsigned oldeip);
void CPU_IRET(bool use32,unsigned oldeip);
void CPU_HLT(unsigned oldeip);

bool CPU_POPF(unsigned use32);
bool CPU_PUSHF(unsigned use32);
bool CPU_CLI(void);
bool CPU_STI(void);

bool CPU_IO_Exception(unsigned port,unsigned size);
void CPU_RunException(void);

void CPU_ENTER(bool use32,unsigned bytes,unsigned level);

#define CPU_INT_SOFTWARE		0x1
#define CPU_INT_EXCEPTION		0x2
#define CPU_INT_HAS_ERROR		0x4
#define CPU_INT_NOIOPLCHECK		0x8

void CPU_Interrupt(unsigned num,unsigned type,unsigned oldeip);
static INLINE void CPU_HW_Interrupt(unsigned num) {
	CPU_Interrupt(num,0,reg_eip);
}
static INLINE void CPU_SW_Interrupt(unsigned num,unsigned oldeip) {
	CPU_Interrupt(num,CPU_INT_SOFTWARE,oldeip);
}
static INLINE void CPU_SW_Interrupt_NoIOPLCheck(unsigned num,unsigned oldeip) {
	CPU_Interrupt(num,CPU_INT_SOFTWARE|CPU_INT_NOIOPLCHECK,oldeip);
}

bool CPU_PrepareException(unsigned which,unsigned error);
void CPU_Exception(unsigned which,unsigned error=0);

bool CPU_SetSegGeneral(SegNames seg,unsigned value);
bool CPU_PopSeg(SegNames seg,bool use32);

bool CPU_CPUID(void);
unsigned CPU_Pop16(void);
unsigned CPU_Pop32(void);
void CPU_Push16(unsigned value);
void CPU_Push32(unsigned value);

void CPU_SetFlags(unsigned word,unsigned mask);


#define EXCEPTION_UD			6
#define EXCEPTION_TS			10
#define EXCEPTION_NP			11
#define EXCEPTION_SS			12
#define EXCEPTION_GP			13
#define EXCEPTION_PF			14

#define CR0_PROTECTION			0x00000001
#define CR0_MONITORPROCESSOR	0x00000002
#define CR0_FPUEMULATION		0x00000004
#define CR0_TASKSWITCH			0x00000008
#define CR0_FPUPRESENT			0x00000010
#define CR0_PAGING				0x80000000


// *********************************************************************
// Descriptor
// *********************************************************************

#define DESC_INVALID				0x00
#define DESC_286_TSS_A				0x01
#define DESC_LDT					0x02
#define DESC_286_TSS_B				0x03
#define DESC_286_CALL_GATE			0x04
#define DESC_TASK_GATE				0x05
#define DESC_286_INT_GATE			0x06
#define DESC_286_TRAP_GATE			0x07

#define DESC_386_TSS_A				0x09
#define DESC_386_TSS_B				0x0b
#define DESC_386_CALL_GATE			0x0c
#define DESC_386_INT_GATE			0x0e
#define DESC_386_TRAP_GATE			0x0f

/* EU/ED Expand UP/DOWN RO/RW Read Only/Read Write NA/A Accessed */
#define DESC_DATA_EU_RO_NA			0x10
#define DESC_DATA_EU_RO_A			0x11
#define DESC_DATA_EU_RW_NA			0x12
#define DESC_DATA_EU_RW_A			0x13
#define DESC_DATA_ED_RO_NA			0x14
#define DESC_DATA_ED_RO_A			0x15
#define DESC_DATA_ED_RW_NA			0x16
#define DESC_DATA_ED_RW_A			0x17

/* N/R Readable  NC/C Confirming A/NA Accessed */
#define DESC_CODE_N_NC_A			0x18
#define DESC_CODE_N_NC_NA			0x19
#define DESC_CODE_R_NC_A			0x1a
#define DESC_CODE_R_NC_NA			0x1b
#define DESC_CODE_N_C_A				0x1c
#define DESC_CODE_N_C_NA			0x1d
#define DESC_CODE_R_C_A				0x1e
#define DESC_CODE_R_C_NA			0x1f

#ifdef _MSC_VER
#pragma pack (1)
#endif

struct S_Descriptor {
#ifdef WORDS_BIGENDIAN
	System::UInt32 base_0_15	:16;
	System::UInt32 limit_0_15	:16;
	System::UInt32 base_24_31	:8;
	System::UInt32 g			:1;
	System::UInt32 big			:1;
	System::UInt32 r			:1;
	System::UInt32 avl			:1;
	System::UInt32 limit_16_19	:4;
	System::UInt32 p			:1;
	System::UInt32 dpl			:2;
	System::UInt32 type			:5;
	System::UInt32 base_16_23	:8;
#else
	System::UInt32 limit_0_15	:16;
	System::UInt32 base_0_15	:16;
	System::UInt32 base_16_23	:8;
	System::UInt32 type			:5;
	System::UInt32 dpl			:2;
	System::UInt32 p			:1;
	System::UInt32 limit_16_19	:4;
	System::UInt32 avl			:1;
	System::UInt32 r			:1;
	System::UInt32 big			:1;
	System::UInt32 g			:1;
	System::UInt32 base_24_31	:8;
#endif
}GCC_ATTRIBUTE(packed);

struct G_Descriptor {
#ifdef WORDS_BIGENDIAN
	System::UInt32 selector:	16;
	System::UInt32 offset_0_15	:16;
	System::UInt32 offset_16_31	:16;
	System::UInt32 p			:1;
	System::UInt32 dpl			:2;
	System::UInt32 type			:5;
	System::UInt32 reserved		:3;
	System::UInt32 paramcount	:5;
#else
	System::UInt32 offset_0_15	:16;
	System::UInt32 selector		:16;
	System::UInt32 paramcount	:5;
	System::UInt32 reserved		:3;
	System::UInt32 type			:5;
	System::UInt32 dpl			:2;
	System::UInt32 p			:1;
	System::UInt32 offset_16_31	:16;
#endif
} GCC_ATTRIBUTE(packed);

struct TSS_16 {	
    System::UInt16 back;                 /* Back link to other task */
    System::UInt16 sp0;				     /* The CK stack pointer */
    System::UInt16 ss0;					 /* The CK stack selector */
	System::UInt16 sp1;                  /* The parent KL stack pointer */
    System::UInt16 ss1;                  /* The parent KL stack selector */
	System::UInt16 sp2;                  /* Unused */
    System::UInt16 ss2;                  /* Unused */
    System::UInt16 ip;                   /* The instruction pointer */
    System::UInt16 flags;                /* The flags */
    System::UInt16 ax, cx, dx, bx;       /* The general purpose registers */
    System::UInt16 sp, bp, si, di;       /* The special purpose registers */
    System::UInt16 es;                   /* The extra selector */
    System::UInt16 cs;                   /* The code selector */
    System::UInt16 ss;                   /* The application stack selector */
    System::UInt16 ds;                   /* The data selector */
    System::UInt16 ldt;                  /* The local descriptor table */
} GCC_ATTRIBUTE(packed);

struct TSS_32 {	
    System::UInt32 back;                /* Back link to other task */
	System::UInt32 esp0;		         /* The CK stack pointer */
    System::UInt32 ss0;					 /* The CK stack selector */
	System::UInt32 esp1;                 /* The parent KL stack pointer */
    System::UInt32 ss1;                  /* The parent KL stack selector */
	System::UInt32 esp2;                 /* Unused */
    System::UInt32 ss2;                  /* Unused */
	System::UInt32 cr3;                  /* The page directory pointer */
    System::UInt32 eip;                  /* The instruction pointer */
    System::UInt32 eflags;               /* The flags */
    System::UInt32 eax, ecx, edx, ebx;   /* The general purpose registers */
    System::UInt32 esp, ebp, esi, edi;   /* The special purpose registers */
    System::UInt32 es;                   /* The extra selector */
    System::UInt32 cs;                   /* The code selector */
    System::UInt32 ss;                   /* The application stack selector */
    System::UInt32 ds;                   /* The data selector */
    System::UInt32 fs;                   /* And another extra selector */
    System::UInt32 gs;                   /* ... and another one */
    System::UInt32 ldt;                  /* The local descriptor table */
} GCC_ATTRIBUTE(packed);

#ifdef _MSC_VER
#pragma pack()
#endif
class Descriptor
{
public:
	Descriptor() { saved.fill[0]=saved.fill[1]=0; }

	void Load(PhysPt address);
	void Save(PhysPt address);

	PhysPt GetBase (void) { 
		return (saved.seg.base_24_31<<24) | (saved.seg.base_16_23<<16) | saved.seg.base_0_15; 
	}
	unsigned GetLimit (void) {
		unsigned limit = (saved.seg.limit_16_19<<16) | saved.seg.limit_0_15;
		if (saved.seg.g)	return (limit<<12) | 0xFFF;
		return limit;
	}
	unsigned GetOffset(void) {
		return (saved.gate.offset_16_31 << 16) | saved.gate.offset_0_15;
	}
	unsigned GetSelector(void) {
		return saved.gate.selector;
	}
	unsigned Type(void) {
		return saved.seg.type;
	}
	unsigned Conforming(void) {
		return saved.seg.type & 8;
	}
	unsigned DPL(void) {
		return saved.seg.dpl;
	}
	unsigned Big(void) {
		return saved.seg.big;
	}
public:
	union {
		S_Descriptor seg;
		G_Descriptor gate;
		System::UInt32 fill[2];
	} saved;
};

class DescriptorTable {
public:
	PhysPt	GetBase			(void)			{ return table_base;	}
	unsigned	GetLimit		(void)			{ return table_limit;	}
	void	SetBase			(PhysPt _base)	{ table_base = _base;	}
	void	SetLimit		(unsigned _limit)	{ table_limit= _limit;	}

	bool GetDescriptor	(unsigned selector, Descriptor& desc) {
		selector&=~7;
		if (selector>=table_limit) return false;
		desc.Load(table_base+(selector));
		return true;
	}
protected:
	PhysPt table_base;
	unsigned table_limit;
};

class GDTDescriptorTable : public DescriptorTable {
public:
	bool GetDescriptor(unsigned selector, Descriptor& desc) {
		unsigned address=selector & ~7;
		if (selector & 4) {
			if (address>=ldt_limit) return false;
			desc.Load(ldt_base+address);
			return true;
		} else {
			if (address>=table_limit) return false;
			desc.Load(table_base+address);
			return true;
		}
	}
	bool SetDescriptor(unsigned selector, Descriptor& desc) {
		unsigned address=selector & ~7;
		if (selector & 4) {
			if (address>=ldt_limit) return false;
			desc.Save(ldt_base+address);
			return true;
		} else {
			if (address>=table_limit) return false;
			desc.Save(table_base+address);
			return true;
		}
	} 
	unsigned SLDT(void)	{
		return ldt_value;
	}
	bool LLDT(unsigned value)	{
		if ((value&0xfffc)==0) {
			ldt_value=0;
			ldt_base=0;
			ldt_limit=0;
			return true;
		}
		Descriptor desc;
		if (!GetDescriptor(value,desc)) return !CPU_PrepareException(EXCEPTION_GP,value);
		if (desc.Type()!=DESC_LDT) return !CPU_PrepareException(EXCEPTION_GP,value);
		if (!desc.saved.seg.p) return !CPU_PrepareException(EXCEPTION_NP,value);
		ldt_base=desc.GetBase();
		ldt_limit=desc.GetLimit();
		ldt_value=value;
		return true;
	}
private:
	PhysPt ldt_base;
	unsigned ldt_limit;
	unsigned ldt_value;
};

class TSS_Descriptor : public Descriptor {
public:
	unsigned IsBusy(void) {
		return saved.seg.type & 2;
	}
	unsigned Is386(void) {
		return saved.seg.type & 8;
	}
	void SetBusy(bool busy) {
		if (busy) saved.seg.type|=2;
		else saved.seg.type&=~2;
	}
};


struct CPUBlock {
	unsigned cpl;							/* Current Privilege */
	unsigned mpl;
	unsigned cr0;
	bool pmode;							/* Is Protected mode enabled */
	GDTDescriptorTable gdt;
	DescriptorTable idt;
	struct {
		unsigned mask,notmask;
		bool big;
	} stack;
	struct {
		bool big;
	} code;
	struct {
		unsigned cs,eip;
		CPU_Decoder * old_decoder;
	} hlt;
	struct {
		unsigned which,error;
	} exception;
	int direction;
	bool trap_skip;
	System::UInt32 drx[8];
	System::UInt32 trx[8];
};

extern CPUBlock cpu;

static INLINE void CPU_SetFlagsd(unsigned word) {
	unsigned mask=cpu.cpl ? FMASK_NORMAL : FMASK_ALL;
	CPU_SetFlags(word,mask);
}

static INLINE void CPU_SetFlagsw(unsigned word) {
	unsigned mask=(cpu.cpl ? FMASK_NORMAL : FMASK_ALL) & 0xffff;
	CPU_SetFlags(word,mask);
}


#endif
