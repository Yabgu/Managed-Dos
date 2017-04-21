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

/* $Id: fpu_instructions.h,v 1.33 2009-05-27 09:15:41 qbix79 Exp $ */


static void FPU_FINIT(void) {
	FPU_SetCW(0x37F);
	fpu.sw = 0;
	TOP=FPU_GET_TOP();
	fpu.tags[0] = TAG_Empty;
	fpu.tags[1] = TAG_Empty;
	fpu.tags[2] = TAG_Empty;
	fpu.tags[3] = TAG_Empty;
	fpu.tags[4] = TAG_Empty;
	fpu.tags[5] = TAG_Empty;
	fpu.tags[6] = TAG_Empty;
	fpu.tags[7] = TAG_Empty;
	fpu.tags[8] = TAG_Valid; // is only used by us
}

static void FPU_FCLEX(void){
	fpu.sw &= 0x7f00;			//should clear exceptions
}

static void FPU_FNOP(void){
	return;
}

static void FPU_PUSH(double in){
	TOP = (TOP - 1) &7;
	//actually check if empty
	fpu.tags[TOP] = TAG_Valid;
	fpu.regs[TOP].d = in;
//	LOG(LOG_FPU,LOG_ERROR)("Pushed at %d  %g to the stack",newtop,in);
	return;
}

static void FPU_PREP_PUSH(void){
	TOP = (TOP - 1) &7;
	fpu.tags[TOP] = TAG_Valid;
}

static void FPU_FPOP(void){
	fpu.tags[TOP]=TAG_Empty;
	//maybe set zero in it as well
	TOP = ((TOP+1)&7);
//	LOG(LOG_FPU,LOG_ERROR)("popped from %d  %g off the stack",top,fpu.regs[top].d);
	return;
}

static double FROUND(double in){
	switch(fpu.round){
	case ROUND_Nearest:	
		if (in-floor(in)>0.5) return (floor(in)+1);
		else if (in-floor(in)<0.5) return (floor(in));
		else return (((static_cast<System::Int64>(floor(in)))&1)!=0)?(floor(in)+1):(floor(in));
		break;
	case ROUND_Down:
		return (floor(in));
		break;
	case ROUND_Up:
		return (ceil(in));
		break;
	case ROUND_Chop:
		return in; //the cast afterwards will do it right maybe cast here
		break;
	default:
		return in;
		break;
	}
}

#define BIAS80 16383
#define BIAS64 1023

static System::Double FPU_FLD80(PhysPt addr) {
	struct {
		System::Int16 begin;
		FPU_Reg eind;
	} test;
	test.eind.l.lower = mem_readd(addr);
	test.eind.l.upper = mem_readd(addr+4);
	test.begin = mem_readw(addr+8);

	System::Int64 exp64 = (((test.begin&0x7fff) - BIAS80));
	System::Int64 blah = ((exp64 >0)?exp64:-exp64)&0x3ff;
	System::Int64 exp64final = ((exp64 >0)?blah:-blah) +BIAS64;

	System::Int64 mant64 = (test.eind.ll >> 11) & LONGTYPE(0xfffffffffffff);
	System::Int64 sign = (test.begin&0x8000)?1:0;
	FPU_Reg result;
	result.ll = (sign <<63)|(exp64final << 52)| mant64;
	return result.d;

	//mant64= test.mant80/2***64    * 2 **53 
}

static void FPU_ST80(PhysPt addr,unsigned reg) {
	struct {
		System::Int16 begin;
		FPU_Reg eind;
	} test;
	System::Int64 sign80 = (fpu.regs[reg].ll&LONGTYPE(0x8000000000000000))?1:0;
	System::Int64 exp80 =  fpu.regs[reg].ll&LONGTYPE(0x7ff0000000000000);
	System::Int64 exp80final = (exp80>>52);
	System::Int64 mant80 = fpu.regs[reg].ll&LONGTYPE(0x000fffffffffffff);
	System::Int64 mant80final = (mant80 << 11);
	if(fpu.regs[reg].d != 0){ //Zero is a special case
		// Elvira wants the 8 and tcalc doesn't
		mant80final |= LONGTYPE(0x8000000000000000);
		//Ca-cyber doesn't like this when result is zero.
		exp80final += (BIAS80 - BIAS64);
	}
	test.begin = (static_cast<System::Int16>(sign80)<<15)| static_cast<System::Int16>(exp80final);
	test.eind.ll = mant80final;
	mem_writed(addr,test.eind.l.lower);
	mem_writed(addr+4,test.eind.l.upper);
	mem_writew(addr+8,test.begin);
}


static void FPU_FLD_F32(PhysPt addr,unsigned store_to) {
	union {
		float f;
		System::UInt32 l;
	}	blah;
	blah.l = mem_readd(addr);
	fpu.regs[store_to].d = static_cast<System::Double>(blah.f);
}

static void FPU_FLD_F64(PhysPt addr,unsigned store_to) {
	fpu.regs[store_to].l.lower = mem_readd(addr);
	fpu.regs[store_to].l.upper = mem_readd(addr+4);
}

static void FPU_FLD_F80(PhysPt addr) {
	fpu.regs[TOP].d = FPU_FLD80(addr);
}

static void FPU_FLD_I16(PhysPt addr,unsigned store_to) {
	System::Int16 blah = mem_readw(addr);
	fpu.regs[store_to].d = static_cast<System::Double>(blah);
}

static void FPU_FLD_I32(PhysPt addr,unsigned store_to) {
	System::Int32 blah = mem_readd(addr);
	fpu.regs[store_to].d = static_cast<System::Double>(blah);
}

static void FPU_FLD_I64(PhysPt addr,unsigned store_to) {
	FPU_Reg blah;
	blah.l.lower = mem_readd(addr);
	blah.l.upper = mem_readd(addr+4);
	fpu.regs[store_to].d = static_cast<System::Double>(blah.ll);
}

static void FPU_FBLD(PhysPt addr,unsigned store_to) {
	System::UInt64 val = 0;
	unsigned in = 0;
	System::UInt64 base = 1;
	for(unsigned i = 0;i < 9;i++){
		in = mem_readb(addr + i);
		val += ( (in&0xf) * base); //in&0xf shouldn't be higher then 9
		base *= 10;
		val += ((( in>>4)&0xf) * base);
		base *= 10;
	}

	//last number, only now convert to float in order to get
	//the best signification
	System::Double temp = static_cast<System::Double>(val);
	in = mem_readb(addr + 9);
	temp += ( (in&0xf) * base );
	if(in&0x80) temp *= -1.0;
	fpu.regs[store_to].d = temp;
}


static INLINE void FPU_FLD_F32_EA(PhysPt addr) {
	FPU_FLD_F32(addr,8);
}
static INLINE void FPU_FLD_F64_EA(PhysPt addr) {
	FPU_FLD_F64(addr,8);
}
static INLINE void FPU_FLD_I32_EA(PhysPt addr) {
	FPU_FLD_I32(addr,8);
}
static INLINE void FPU_FLD_I16_EA(PhysPt addr) {
	FPU_FLD_I16(addr,8);
}


static void FPU_FST_F32(PhysPt addr) {
	union {
		float f;
		System::UInt32 l;
	}	blah;
	//should depend on rounding method
	blah.f = static_cast<float>(fpu.regs[TOP].d);
	mem_writed(addr,blah.l);
}

static void FPU_FST_F64(PhysPt addr) {
	mem_writed(addr,fpu.regs[TOP].l.lower);
	mem_writed(addr+4,fpu.regs[TOP].l.upper);
}

static void FPU_FST_F80(PhysPt addr) {
	FPU_ST80(addr,TOP);
}

static void FPU_FST_I16(PhysPt addr) {
	mem_writew(addr,static_cast<System::Int16>(FROUND(fpu.regs[TOP].d)));
}

static void FPU_FST_I32(PhysPt addr) {
	mem_writed(addr,static_cast<System::Int32>(FROUND(fpu.regs[TOP].d)));
}

static void FPU_FST_I64(PhysPt addr) {
	FPU_Reg blah;
	blah.ll = static_cast<System::Int64>(FROUND(fpu.regs[TOP].d));
	mem_writed(addr,blah.l.lower);
	mem_writed(addr+4,blah.l.upper);
}

static void FPU_FBST(PhysPt addr) {
	FPU_Reg val = fpu.regs[TOP];
	bool sign = false;
	if(fpu.regs[TOP].ll & LONGTYPE(0x8000000000000000)) { //sign
		sign=true;
		val.d=-val.d;
	}
	//numbers from back to front
	System::Double temp=val.d;
	unsigned p;
	for(unsigned i=0;i<9;i++){
		val.d=temp;
		temp = static_cast<System::Double>(static_cast<System::Int64>(floor(val.d/10.0)));
		p = static_cast<unsigned>(val.d - 10.0*temp);  
		val.d=temp;
		temp = static_cast<System::Double>(static_cast<System::Int64>(floor(val.d/10.0)));
		p |= (static_cast<unsigned>(val.d - 10.0*temp)<<4);

		mem_writeb(addr+i,p);
	}
	val.d=temp;
	temp = static_cast<System::Double>(static_cast<System::Int64>(floor(val.d/10.0)));
	p = static_cast<unsigned>(val.d - 10.0*temp);
	if(sign)
		p|=0x80;
	mem_writeb(addr+9,p);
}

static void FPU_FADD(unsigned op1, unsigned op2){
	fpu.regs[op1].d+=fpu.regs[op2].d;
	//flags and such :)
	return;
}

static void FPU_FSIN(void){
	fpu.regs[TOP].d = sin(fpu.regs[TOP].d);
	FPU_SET_C2(0);
	//flags and such :)
	return;
}

static void FPU_FSINCOS(void){
	System::Double temp = fpu.regs[TOP].d;
	fpu.regs[TOP].d = sin(temp);
	FPU_PUSH(cos(temp));
	FPU_SET_C2(0);
	//flags and such :)
	return;
}

static void FPU_FCOS(void){
	fpu.regs[TOP].d = cos(fpu.regs[TOP].d);
	FPU_SET_C2(0);
	//flags and such :)
	return;
}

static void FPU_FSQRT(void){
	fpu.regs[TOP].d = sqrt(fpu.regs[TOP].d);
	//flags and such :)
	return;
}
static void FPU_FPATAN(void){
	fpu.regs[STV(1)].d = atan2(fpu.regs[STV(1)].d,fpu.regs[TOP].d);
	FPU_FPOP();
	//flags and such :)
	return;
}
static void FPU_FPTAN(void){
	fpu.regs[TOP].d = tan(fpu.regs[TOP].d);
	FPU_PUSH(1.0);
	FPU_SET_C2(0);
	//flags and such :)
	return;
}
static void FPU_FDIV(unsigned st, unsigned other){
	fpu.regs[st].d= fpu.regs[st].d/fpu.regs[other].d;
	//flags and such :)
	return;
}

static void FPU_FDIVR(unsigned st, unsigned other){
	fpu.regs[st].d= fpu.regs[other].d/fpu.regs[st].d;
	// flags and such :)
	return;
}

static void FPU_FMUL(unsigned st, unsigned other){
	fpu.regs[st].d*=fpu.regs[other].d;
	//flags and such :)
	return;
}

static void FPU_FSUB(unsigned st, unsigned other){
	fpu.regs[st].d = fpu.regs[st].d - fpu.regs[other].d;
	//flags and such :)
	return;
}

static void FPU_FSUBR(unsigned st, unsigned other){
	fpu.regs[st].d= fpu.regs[other].d - fpu.regs[st].d;
	//flags and such :)
	return;
}

static void FPU_FXCH(unsigned st, unsigned other){
	FPU_Tag tag = fpu.tags[other];
	FPU_Reg reg = fpu.regs[other];
	fpu.tags[other] = fpu.tags[st];
	fpu.regs[other] = fpu.regs[st];
	fpu.tags[st] = tag;
	fpu.regs[st] = reg;
}

static void FPU_FST(unsigned st, unsigned other){
	fpu.tags[other] = fpu.tags[st];
	fpu.regs[other] = fpu.regs[st];
}


static void FPU_FCOM(unsigned st, unsigned other){
	if(((fpu.tags[st] != TAG_Valid) && (fpu.tags[st] != TAG_Zero)) || 
		((fpu.tags[other] != TAG_Valid) && (fpu.tags[other] != TAG_Zero))){
		FPU_SET_C3(1);FPU_SET_C2(1);FPU_SET_C0(1);return;
	}
	if(fpu.regs[st].d == fpu.regs[other].d){
		FPU_SET_C3(1);FPU_SET_C2(0);FPU_SET_C0(0);return;
	}
	if(fpu.regs[st].d < fpu.regs[other].d){
		FPU_SET_C3(0);FPU_SET_C2(0);FPU_SET_C0(1);return;
	}
	// st > other
	FPU_SET_C3(0);FPU_SET_C2(0);FPU_SET_C0(0);return;
}

static void FPU_FUCOM(unsigned st, unsigned other){
	//does atm the same as fcom 
	FPU_FCOM(st,other);
}

static void FPU_FRNDINT(void){
	System::Int64 temp= static_cast<System::Int64>(FROUND(fpu.regs[TOP].d));
	fpu.regs[TOP].d=static_cast<double>(temp);
}

static void FPU_FPREM(void){
	System::Double valtop = fpu.regs[TOP].d;
	System::Double valdiv = fpu.regs[STV(1)].d;
	System::Int64 ressaved = static_cast<System::Int64>( (valtop/valdiv) );
// Some backups
//	System::Double res=valtop - ressaved*valdiv; 
//      res= fmod(valtop,valdiv);
	fpu.regs[TOP].d = valtop - ressaved*valdiv;
	FPU_SET_C0(static_cast<unsigned>(ressaved&4));
	FPU_SET_C3(static_cast<unsigned>(ressaved&2));
	FPU_SET_C1(static_cast<unsigned>(ressaved&1));
	FPU_SET_C2(0);
}

static void FPU_FPREM1(void){
	System::Double valtop = fpu.regs[TOP].d;
	System::Double valdiv = fpu.regs[STV(1)].d;
	double quot = valtop/valdiv;
	double quotf = floor(quot);
	System::Int64 ressaved;
	if (quot-quotf>0.5) ressaved = static_cast<System::Int64>(quotf+1);
	else if (quot-quotf<0.5) ressaved = static_cast<System::Int64>(quotf);
	else ressaved = static_cast<System::Int64>((((static_cast<System::Int64>(quotf))&1)!=0)?(quotf+1):(quotf));
	fpu.regs[TOP].d = valtop - ressaved*valdiv;
	FPU_SET_C0(static_cast<unsigned>(ressaved&4));
	FPU_SET_C3(static_cast<unsigned>(ressaved&2));
	FPU_SET_C1(static_cast<unsigned>(ressaved&1));
	FPU_SET_C2(0);
}

static void FPU_FXAM(void){
	if(fpu.regs[TOP].ll & LONGTYPE(0x8000000000000000))	//sign
	{ 
		FPU_SET_C1(1);
	} 
	else 
	{
		FPU_SET_C1(0);
	}
	if(fpu.tags[TOP] == TAG_Empty)
	{
		FPU_SET_C3(1);FPU_SET_C2(0);FPU_SET_C0(1);
		return;
	}
	if(fpu.regs[TOP].d == 0.0)		//zero or normalized number.
	{ 
		FPU_SET_C3(1);FPU_SET_C2(0);FPU_SET_C0(0);
	}
	else
	{
		FPU_SET_C3(0);FPU_SET_C2(1);FPU_SET_C0(0);
	}
}


static void FPU_F2XM1(void){
	fpu.regs[TOP].d = pow(2.0,fpu.regs[TOP].d) - 1;
	return;
}

static void FPU_FYL2X(void){
	fpu.regs[STV(1)].d*=log(fpu.regs[TOP].d)/log(static_cast<System::Double>(2.0));
	FPU_FPOP();
	return;
}

static void FPU_FYL2XP1(void){
	fpu.regs[STV(1)].d*=log(fpu.regs[TOP].d+1.0)/log(static_cast<System::Double>(2.0));
	FPU_FPOP();
	return;
}

static void FPU_FSCALE(void){
	fpu.regs[TOP].d *= pow(2.0,static_cast<System::Double>(static_cast<System::Int64>(fpu.regs[STV(1)].d)));
	return; //2^x where x is chopped.
}

static void FPU_FSTENV(PhysPt addr){
	FPU_SET_TOP(TOP);
	if(!cpu.code.big) {
		mem_writew(addr+0,static_cast<System::UInt16>(fpu.cw));
		mem_writew(addr+2,static_cast<System::UInt16>(fpu.sw));
		mem_writew(addr+4,static_cast<System::UInt16>(FPU_GetTag()));
	} else { 
		mem_writed(addr+0,static_cast<System::UInt32>(fpu.cw));
		mem_writed(addr+4,static_cast<System::UInt32>(fpu.sw));
		mem_writed(addr+8,static_cast<System::UInt32>(FPU_GetTag()));
	}
}

static void FPU_FLDENV(PhysPt addr){
	System::UInt16 tag;
	System::UInt32 tagbig;
	unsigned cw;
	if(!cpu.code.big) {
		cw     = mem_readw(addr+0);
		fpu.sw = mem_readw(addr+2);
		tag    = mem_readw(addr+4);
	} else { 
		cw     = mem_readd(addr+0);
		fpu.sw = (System::UInt16)mem_readd(addr+4);
		tagbig = mem_readd(addr+8);
		tag    = static_cast<System::UInt16>(tagbig);
	}
	FPU_SetTag(tag);
	FPU_SetCW(cw);
	TOP = FPU_GET_TOP();
}

static void FPU_FSAVE(PhysPt addr){
	FPU_FSTENV(addr);
	unsigned start = (cpu.code.big?28:14);
	for(unsigned i = 0;i < 8;i++){
		FPU_ST80(addr+start,STV(i));
		start += 10;
	}
	FPU_FINIT();
}

static void FPU_FRSTOR(PhysPt addr){
	FPU_FLDENV(addr);
	unsigned start = (cpu.code.big?28:14);
	for(unsigned i = 0;i < 8;i++){
		fpu.regs[STV(i)].d = FPU_FLD80(addr+start);
		start += 10;
	}
}

static void FPU_FXTRACT(void) {
	// function stores real bias in st and 
	// pushes the significant number onto the stack
	// if double ever uses a different base please correct this function

	FPU_Reg test = fpu.regs[TOP];
	System::Int64 exp80 =  test.ll&LONGTYPE(0x7ff0000000000000);
	System::Int64 exp80final = (exp80>>52) - BIAS64;
	System::Double mant = test.d / (pow(2.0,static_cast<System::Double>(exp80final)));
	fpu.regs[TOP].d = static_cast<System::Double>(exp80final);
	FPU_PUSH(mant);
}

static void FPU_FCHS(void){
	fpu.regs[TOP].d = -1.0*(fpu.regs[TOP].d);
}

static void FPU_FABS(void){
	fpu.regs[TOP].d = fabs(fpu.regs[TOP].d);
}

static void FPU_FTST(void){
	fpu.regs[8].d = 0.0;
	FPU_FCOM(TOP,8);
}

static void FPU_FLD1(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = 1.0;
}

static void FPU_FLDL2T(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = L2T;
}

static void FPU_FLDL2E(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = L2E;
}

static void FPU_FLDPI(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = PI;
}

static void FPU_FLDLG2(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = LG2;
}

static void FPU_FLDLN2(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = LN2;
}

static void FPU_FLDZ(void){
	FPU_PREP_PUSH();
	fpu.regs[TOP].d = 0.0;
	fpu.tags[TOP] = TAG_Zero;
}


static INLINE void FPU_FADD_EA(unsigned op1){
	FPU_FADD(op1,8);
}
static INLINE void FPU_FMUL_EA(unsigned op1){
	FPU_FMUL(op1,8);
}
static INLINE void FPU_FSUB_EA(unsigned op1){
	FPU_FSUB(op1,8);
}
static INLINE void FPU_FSUBR_EA(unsigned op1){
	FPU_FSUBR(op1,8);
}
static INLINE void FPU_FDIV_EA(unsigned op1){
	FPU_FDIV(op1,8);
}
static INLINE void FPU_FDIVR_EA(unsigned op1){
	FPU_FDIVR(op1,8);
}
static INLINE void FPU_FCOM_EA(unsigned op1){
	FPU_FCOM(op1,8);
}

