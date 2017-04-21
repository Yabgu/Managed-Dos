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

/* $Id: operators.h,v 1.8 2009-06-25 19:31:43 c2woody Exp $ */


static System::Byte DRC_CALL_CONV dynrec_add_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_add_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=(System::Byte)(lf_var1b+lf_var2b);
	lflags.type=t_ADDb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_add_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_add_byte_simple(System::Byte op1,System::Byte op2) {
	return op1+op2;
}

static System::Byte DRC_CALL_CONV dynrec_adc_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_adc_byte(System::Byte op1,System::Byte op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=(System::Byte)(lf_var1b+lf_var2b+lflags.oldcf);
	lflags.type=t_ADCb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_adc_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_adc_byte_simple(System::Byte op1,System::Byte op2) {
	return (System::Byte)(op1+op2+(unsigned)(get_CF()!=0));
}

static System::Byte DRC_CALL_CONV dynrec_sub_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sub_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=(System::Byte)(lf_var1b-lf_var2b);
	lflags.type=t_SUBb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_sub_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sub_byte_simple(System::Byte op1,System::Byte op2) {
	return op1-op2;
}

static System::Byte DRC_CALL_CONV dynrec_sbb_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sbb_byte(System::Byte op1,System::Byte op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=(System::Byte)(lf_var1b-(lf_var2b+lflags.oldcf));
	lflags.type=t_SBBb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_sbb_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sbb_byte_simple(System::Byte op1,System::Byte op2) {
	return (System::Byte)(op1-(op2+(unsigned)(get_CF()!=0)));
}

static void DRC_CALL_CONV dynrec_cmp_byte(System::Byte op1,System::Byte op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=(System::Byte)(lf_var1b-lf_var2b);
	lflags.type=t_CMPb;
}

static void DRC_CALL_CONV dynrec_cmp_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_byte_simple(System::Byte op1,System::Byte op2) {
}

static System::Byte DRC_CALL_CONV dynrec_xor_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_xor_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b ^ lf_var2b;
	lflags.type=t_XORb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_xor_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_xor_byte_simple(System::Byte op1,System::Byte op2) {
	return op1 ^ op2;
}

static System::Byte DRC_CALL_CONV dynrec_and_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_and_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b & lf_var2b;
	lflags.type=t_ANDb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_and_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_and_byte_simple(System::Byte op1,System::Byte op2) {
	return op1 & op2;
}

static System::Byte DRC_CALL_CONV dynrec_or_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_or_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b | lf_var2b;
	lflags.type=t_ORb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_or_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_or_byte_simple(System::Byte op1,System::Byte op2) {
	return op1 | op2;
}

static void DRC_CALL_CONV dynrec_test_byte(System::Byte op1,System::Byte op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_byte(System::Byte op1,System::Byte op2) {
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b & lf_var2b;
	lflags.type=t_TESTb;
}

static void DRC_CALL_CONV dynrec_test_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_byte_simple(System::Byte op1,System::Byte op2) {
}

static System::UInt16 DRC_CALL_CONV dynrec_add_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_add_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=(System::UInt16)(lf_var1w+lf_var2w);
	lflags.type=t_ADDw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_add_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_add_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return op1+op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_adc_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_adc_word(System::UInt16 op1,System::UInt16 op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=(System::UInt16)(lf_var1w+lf_var2w+lflags.oldcf);
	lflags.type=t_ADCw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_adc_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_adc_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return (System::UInt16)(op1+op2+(unsigned)(get_CF()!=0));
}

static System::UInt16 DRC_CALL_CONV dynrec_sub_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sub_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=(System::UInt16)(lf_var1w-lf_var2w);
	lflags.type=t_SUBw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_sub_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sub_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return op1-op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_sbb_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sbb_word(System::UInt16 op1,System::UInt16 op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=(System::UInt16)(lf_var1w-(lf_var2w+lflags.oldcf));
	lflags.type=t_SBBw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_sbb_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sbb_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return (System::UInt16)(op1-(op2+(unsigned)(get_CF()!=0)));
}

static void DRC_CALL_CONV dynrec_cmp_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=(System::UInt16)(lf_var1w-lf_var2w);
	lflags.type=t_CMPw;
}

static void DRC_CALL_CONV dynrec_cmp_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_word_simple(System::UInt16 op1,System::UInt16 op2) {
}

static System::UInt16 DRC_CALL_CONV dynrec_xor_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_xor_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=lf_var1w ^ lf_var2w;
	lflags.type=t_XORw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_xor_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_xor_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return op1 ^ op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_and_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_and_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=lf_var1w & lf_var2w;
	lflags.type=t_ANDw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_and_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_and_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return op1 & op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_or_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_or_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=lf_var1w | lf_var2w;
	lflags.type=t_ORw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_or_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_or_word_simple(System::UInt16 op1,System::UInt16 op2) {
	return op1 | op2;
}

static void DRC_CALL_CONV dynrec_test_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_word(System::UInt16 op1,System::UInt16 op2) {
	lf_var1w=op1;
	lf_var2w=op2;
	lf_resw=lf_var1w & lf_var2w;
	lflags.type=t_TESTw;
}

static void DRC_CALL_CONV dynrec_test_word_simple(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_word_simple(System::UInt16 op1,System::UInt16 op2) {
}

static System::UInt32 DRC_CALL_CONV dynrec_add_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_add_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d+lf_var2d;
	lflags.type=t_ADDd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_add_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_add_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1 + op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_adc_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_adc_dword(System::UInt32 op1,System::UInt32 op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d+lf_var2d+lflags.oldcf;
	lflags.type=t_ADCd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_adc_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_adc_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1+op2+(unsigned)(get_CF()!=0);
}

static System::UInt32 DRC_CALL_CONV dynrec_sub_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sub_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d-lf_var2d;
	lflags.type=t_SUBd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_sub_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sub_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1-op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_sbb_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sbb_dword(System::UInt32 op1,System::UInt32 op2) {
	lflags.oldcf=get_CF()!=0;
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d-(lf_var2d+lflags.oldcf);
	lflags.type=t_SBBd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_sbb_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sbb_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1-(op2+(unsigned)(get_CF()!=0));
}

static void DRC_CALL_CONV dynrec_cmp_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d-lf_var2d;
	lflags.type=t_CMPd;
}

static void DRC_CALL_CONV dynrec_cmp_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_cmp_dword_simple(System::UInt32 op1,System::UInt32 op2) {
}

static System::UInt32 DRC_CALL_CONV dynrec_xor_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_xor_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d ^ lf_var2d;
	lflags.type=t_XORd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_xor_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_xor_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1 ^ op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_and_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_and_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d & lf_var2d;
	lflags.type=t_ANDd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_and_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_and_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1 & op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_or_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_or_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d | lf_var2d;
	lflags.type=t_ORd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_or_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_or_dword_simple(System::UInt32 op1,System::UInt32 op2) {
	return op1 | op2;
}

static void DRC_CALL_CONV dynrec_test_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_dword(System::UInt32 op1,System::UInt32 op2) {
	lf_var1d=op1;
	lf_var2d=op2;
	lf_resd=lf_var1d & lf_var2d;
	lflags.type=t_TESTd;
}

static void DRC_CALL_CONV dynrec_test_dword_simple(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static void DRC_CALL_CONV dynrec_test_dword_simple(System::UInt32 op1,System::UInt32 op2) {
}


static void dyn_dop_byte_gencall(DualOps op) {
	switch (op) {
		case DOP_ADD:
			InvalidateFlags((void*)&dynrec_add_byte_simple,t_ADDb);
			gen_call_function_raw((void*)&dynrec_add_byte);
			break;
		case DOP_ADC:
			AcquireFlags(FLAG_CF);
			InvalidateFlagsPartially((void*)&dynrec_adc_byte_simple,t_ADCb);
			gen_call_function_raw((void*)&dynrec_adc_byte);
			break;
		case DOP_SUB:
			InvalidateFlags((void*)&dynrec_sub_byte_simple,t_SUBb);
			gen_call_function_raw((void*)&dynrec_sub_byte);
			break;
		case DOP_SBB:
			AcquireFlags(FLAG_CF);
			InvalidateFlagsPartially((void*)&dynrec_sbb_byte_simple,t_SBBb);
			gen_call_function_raw((void*)&dynrec_sbb_byte);
			break;
		case DOP_CMP:
			InvalidateFlags((void*)&dynrec_cmp_byte_simple,t_CMPb);
			gen_call_function_raw((void*)&dynrec_cmp_byte);
			break;
		case DOP_XOR:
			InvalidateFlags((void*)&dynrec_xor_byte_simple,t_XORb);
			gen_call_function_raw((void*)&dynrec_xor_byte);
			break;
		case DOP_AND:
			InvalidateFlags((void*)&dynrec_and_byte_simple,t_ANDb);
			gen_call_function_raw((void*)&dynrec_and_byte);
			break;
		case DOP_OR:
			InvalidateFlags((void*)&dynrec_or_byte_simple,t_ORb);
			gen_call_function_raw((void*)&dynrec_or_byte);
			break;
		case DOP_TEST:
			InvalidateFlags((void*)&dynrec_test_byte_simple,t_TESTb);
			gen_call_function_raw((void*)&dynrec_test_byte);
			break;
		default: IllegalOptionDynrec("dyn_dop_byte_gencall");
	}
}

static void dyn_dop_word_gencall(DualOps op,bool dword) {
	if (dword) {
		switch (op) {
			case DOP_ADD:
				InvalidateFlags((void*)&dynrec_add_dword_simple,t_ADDd);
				gen_call_function_raw((void*)&dynrec_add_dword);
				break;
			case DOP_ADC:
				AcquireFlags(FLAG_CF);
				InvalidateFlagsPartially((void*)&dynrec_adc_dword_simple,t_ADCd);
				gen_call_function_raw((void*)&dynrec_adc_dword);
				break;
			case DOP_SUB:
				InvalidateFlags((void*)&dynrec_sub_dword_simple,t_SUBd);
				gen_call_function_raw((void*)&dynrec_sub_dword);
				break;
			case DOP_SBB:
				AcquireFlags(FLAG_CF);
				InvalidateFlagsPartially((void*)&dynrec_sbb_dword_simple,t_SBBd);
				gen_call_function_raw((void*)&dynrec_sbb_dword);
				break;
			case DOP_CMP:
				InvalidateFlags((void*)&dynrec_cmp_dword_simple,t_CMPd);
				gen_call_function_raw((void*)&dynrec_cmp_dword);
				break;
			case DOP_XOR:
				InvalidateFlags((void*)&dynrec_xor_dword_simple,t_XORd);
				gen_call_function_raw((void*)&dynrec_xor_dword);
				break;
			case DOP_AND:
				InvalidateFlags((void*)&dynrec_and_dword_simple,t_ANDd);
				gen_call_function_raw((void*)&dynrec_and_dword);
				break;
			case DOP_OR:
				InvalidateFlags((void*)&dynrec_or_dword_simple,t_ORd);
				gen_call_function_raw((void*)&dynrec_or_dword);
				break;
			case DOP_TEST:
				InvalidateFlags((void*)&dynrec_test_dword_simple,t_TESTd);
				gen_call_function_raw((void*)&dynrec_test_dword);
				break;
			default: IllegalOptionDynrec("dyn_dop_dword_gencall");
		}
	} else {
		switch (op) {
			case DOP_ADD:
				InvalidateFlags((void*)&dynrec_add_word_simple,t_ADDw);
				gen_call_function_raw((void*)&dynrec_add_word);
				break;
			case DOP_ADC:
				AcquireFlags(FLAG_CF);
				InvalidateFlagsPartially((void*)&dynrec_adc_word_simple,t_ADCw);
				gen_call_function_raw((void*)&dynrec_adc_word);
				break;
			case DOP_SUB:
				InvalidateFlags((void*)&dynrec_sub_word_simple,t_SUBw);
				gen_call_function_raw((void*)&dynrec_sub_word);
				break;
			case DOP_SBB:
				AcquireFlags(FLAG_CF);
				InvalidateFlagsPartially((void*)&dynrec_sbb_word_simple,t_SBBw);
				gen_call_function_raw((void*)&dynrec_sbb_word);
				break;
			case DOP_CMP:
				InvalidateFlags((void*)&dynrec_cmp_word_simple,t_CMPw);
				gen_call_function_raw((void*)&dynrec_cmp_word);
				break;
			case DOP_XOR:
				InvalidateFlags((void*)&dynrec_xor_word_simple,t_XORw);
				gen_call_function_raw((void*)&dynrec_xor_word);
				break;
			case DOP_AND:
				InvalidateFlags((void*)&dynrec_and_word_simple,t_ANDw);
				gen_call_function_raw((void*)&dynrec_and_word);
				break;
			case DOP_OR:
				InvalidateFlags((void*)&dynrec_or_word_simple,t_ORw);
				gen_call_function_raw((void*)&dynrec_or_word);
				break;
			case DOP_TEST:
				InvalidateFlags((void*)&dynrec_test_word_simple,t_TESTw);
				gen_call_function_raw((void*)&dynrec_test_word);
				break;
			default: IllegalOptionDynrec("dyn_dop_word_gencall");
		}
	}
}


static System::Byte DRC_CALL_CONV dynrec_inc_byte(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_inc_byte(System::Byte op) {
	LoadCF;
	lf_var1b=op;
	lf_resb=lf_var1b+1;
	lflags.type=t_INCb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_inc_byte_simple(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_inc_byte_simple(System::Byte op) {
	return op+1;
}

static System::Byte DRC_CALL_CONV dynrec_dec_byte(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_dec_byte(System::Byte op) {
	LoadCF;
	lf_var1b=op;
	lf_resb=lf_var1b-1;
	lflags.type=t_DECb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_dec_byte_simple(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_dec_byte_simple(System::Byte op) {
	return op-1;
}

static System::Byte DRC_CALL_CONV dynrec_not_byte(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_not_byte(System::Byte op) {
	return ~op;
}

static System::Byte DRC_CALL_CONV dynrec_neg_byte(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_neg_byte(System::Byte op) {
	lf_var1b=op;
	lf_resb=0-lf_var1b;
	lflags.type=t_NEGb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_neg_byte_simple(System::Byte op) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_neg_byte_simple(System::Byte op) {
	return 0-op;
}

static System::UInt16 DRC_CALL_CONV dynrec_inc_word(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_inc_word(System::UInt16 op) {
	LoadCF;
	lf_var1w=op;
	lf_resw=lf_var1w+1;
	lflags.type=t_INCw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_inc_word_simple(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_inc_word_simple(System::UInt16 op) {
	return op+1;
}

static System::UInt16 DRC_CALL_CONV dynrec_dec_word(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dec_word(System::UInt16 op) {
	LoadCF;
	lf_var1w=op;
	lf_resw=lf_var1w-1;
	lflags.type=t_DECw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_dec_word_simple(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dec_word_simple(System::UInt16 op) {
	return op-1;
}

static System::UInt16 DRC_CALL_CONV dynrec_not_word(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_not_word(System::UInt16 op) {
	return ~op;
}

static System::UInt16 DRC_CALL_CONV dynrec_neg_word(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_neg_word(System::UInt16 op) {
	lf_var1w=op;
	lf_resw=0-lf_var1w;
	lflags.type=t_NEGw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_neg_word_simple(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_neg_word_simple(System::UInt16 op) {
	return 0-op;
}

static System::UInt32 DRC_CALL_CONV dynrec_inc_dword(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_inc_dword(System::UInt32 op) {
	LoadCF;
	lf_var1d=op;
	lf_resd=lf_var1d+1;
	lflags.type=t_INCd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_inc_dword_simple(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_inc_dword_simple(System::UInt32 op) {
	return op+1;
}

static System::UInt32 DRC_CALL_CONV dynrec_dec_dword(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dec_dword(System::UInt32 op) {
	LoadCF;
	lf_var1d=op;
	lf_resd=lf_var1d-1;
	lflags.type=t_DECd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_dec_dword_simple(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dec_dword_simple(System::UInt32 op) {
	return op-1;
}

static System::UInt32 DRC_CALL_CONV dynrec_not_dword(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_not_dword(System::UInt32 op) {
	return ~op;
}

static System::UInt32 DRC_CALL_CONV dynrec_neg_dword(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_neg_dword(System::UInt32 op) {
	lf_var1d=op;
	lf_resd=0-lf_var1d;
	lflags.type=t_NEGd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_neg_dword_simple(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_neg_dword_simple(System::UInt32 op) {
	return 0-op;
}


static void dyn_sop_byte_gencall(SingleOps op) {
	switch (op) {
		case SOP_INC:
			InvalidateFlagsPartially((void*)&dynrec_inc_byte_simple,t_INCb);
			gen_call_function_raw((void*)&dynrec_inc_byte);
			break;
		case SOP_DEC:
			InvalidateFlagsPartially((void*)&dynrec_dec_byte_simple,t_DECb);
			gen_call_function_raw((void*)&dynrec_dec_byte);
			break;
		case SOP_NOT:
			gen_call_function_raw((void*)&dynrec_not_byte);
			break;
		case SOP_NEG:
			InvalidateFlags((void*)&dynrec_neg_byte_simple,t_NEGb);
			gen_call_function_raw((void*)&dynrec_neg_byte);
			break;
		default: IllegalOptionDynrec("dyn_sop_byte_gencall");
	}
}

static void dyn_sop_word_gencall(SingleOps op,bool dword) {
	if (dword) {
		switch (op) {
			case SOP_INC:
				InvalidateFlagsPartially((void*)&dynrec_inc_dword_simple,t_INCd);
				gen_call_function_raw((void*)&dynrec_inc_dword);
				break;
			case SOP_DEC:
				InvalidateFlagsPartially((void*)&dynrec_dec_dword_simple,t_DECd);
				gen_call_function_raw((void*)&dynrec_dec_dword);
				break;
			case SOP_NOT:
				gen_call_function_raw((void*)&dynrec_not_dword);
				break;
			case SOP_NEG:
				InvalidateFlags((void*)&dynrec_neg_dword_simple,t_NEGd);
				gen_call_function_raw((void*)&dynrec_neg_dword);
				break;
			default: IllegalOptionDynrec("dyn_sop_dword_gencall");
		}
	} else {
		switch (op) {
			case SOP_INC:
				InvalidateFlagsPartially((void*)&dynrec_inc_word_simple,t_INCw);
				gen_call_function_raw((void*)&dynrec_inc_word);
				break;
			case SOP_DEC:
				InvalidateFlagsPartially((void*)&dynrec_dec_word_simple,t_DECw);
				gen_call_function_raw((void*)&dynrec_dec_word);
				break;
			case SOP_NOT:
				gen_call_function_raw((void*)&dynrec_not_word);
				break;
			case SOP_NEG:
				InvalidateFlags((void*)&dynrec_neg_word_simple,t_NEGw);
				gen_call_function_raw((void*)&dynrec_neg_word);
				break;
			default: IllegalOptionDynrec("dyn_sop_word_gencall");
		}
	}
}


static System::Byte DRC_CALL_CONV dynrec_rol_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_rol_byte(System::Byte op1,System::Byte op2) {
	if (!(op2&0x7)) {
		if (op2&0x18) {
			FillFlagsNoCFOF();
			SETFLAGBIT(CF,op1 & 1);
			SETFLAGBIT(OF,(op1 & 1) ^ (op1 >> 7));
		}
		return op1;
	}
	FillFlagsNoCFOF();
	lf_var1b=op1;
	lf_var2b=op2&0x07;
	lf_resb=(lf_var1b << lf_var2b) | (lf_var1b >> (8-lf_var2b));
	SETFLAGBIT(CF,lf_resb & 1);
	SETFLAGBIT(OF,(lf_resb & 1) ^ (lf_resb >> 7));
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_rol_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_rol_byte_simple(System::Byte op1,System::Byte op2) {
	if (!(op2&0x7)) return op1;
	return (op1 << (op2&0x07)) | (op1 >> (8-(op2&0x07)));
}

static System::Byte DRC_CALL_CONV dynrec_ror_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_ror_byte(System::Byte op1,System::Byte op2) {
	if (!(op2&0x7)) {
		if (op2&0x18) {
			FillFlagsNoCFOF();
			SETFLAGBIT(CF,op1>>7);
			SETFLAGBIT(OF,(op1>>7) ^ ((op1>>6) & 1));
		}
		return op1;
	}
	FillFlagsNoCFOF();
	lf_var1b=op1;
	lf_var2b=op2&0x07;
	lf_resb=(lf_var1b >> lf_var2b) | (lf_var1b << (8-lf_var2b));
	SETFLAGBIT(CF,lf_resb & 0x80);
	SETFLAGBIT(OF,(lf_resb ^ (lf_resb<<1)) & 0x80);
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_ror_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_ror_byte_simple(System::Byte op1,System::Byte op2) {
	if (!(op2&0x7)) return op1;
	return (op1 >> (op2&0x07)) | (op1 << (8-(op2&0x07)));
}

static System::Byte DRC_CALL_CONV dynrec_rcl_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_rcl_byte(System::Byte op1,System::Byte op2) {
	if (op2%9) {
		System::Byte cf=(System::Byte)FillFlags()&0x1;
		lf_var1b=op1;
		lf_var2b=op2%9;
		lf_resb=(lf_var1b << lf_var2b) | (cf << (lf_var2b-1)) | (lf_var1b >> (9-lf_var2b));
		SETFLAGBIT(CF,((lf_var1b >> (8-lf_var2b)) & 1));
		SETFLAGBIT(OF,(reg_flags & 1) ^ (lf_resb >> 7));
		return lf_resb;
	} else return op1;
}

static System::Byte DRC_CALL_CONV dynrec_rcr_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_rcr_byte(System::Byte op1,System::Byte op2) {
	if (op2%9) {
		System::Byte cf=(System::Byte)FillFlags()&0x1;
		lf_var1b=op1;
		lf_var2b=op2%9;
	 	lf_resb=(lf_var1b >> lf_var2b) | (cf << (8-lf_var2b)) | (lf_var1b << (9-lf_var2b));					\
		SETFLAGBIT(CF,(lf_var1b >> (lf_var2b - 1)) & 1);
		SETFLAGBIT(OF,(lf_resb ^ (lf_resb<<1)) & 0x80);
		return lf_resb;
	} else return op1;
}

static System::Byte DRC_CALL_CONV dynrec_shl_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_shl_byte(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b << lf_var2b;
	lflags.type=t_SHLb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_shl_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_shl_byte_simple(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 << op2;
}

static System::Byte DRC_CALL_CONV dynrec_shr_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_shr_byte(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1b=op1;
	lf_var2b=op2;
	lf_resb=lf_var1b >> lf_var2b;
	lflags.type=t_SHRb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_shr_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_shr_byte_simple(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 >> op2;
}

static System::Byte DRC_CALL_CONV dynrec_sar_byte(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sar_byte(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1b=op1;
	lf_var2b=op2;
	if (lf_var2b>8) lf_var2b=8;
    if (lf_var1b & 0x80) {
		lf_resb=(lf_var1b >> lf_var2b)| (0xff << (8 - lf_var2b));
	} else {
		lf_resb=lf_var1b >> lf_var2b;
    }
	lflags.type=t_SARb;
	return lf_resb;
}

static System::Byte DRC_CALL_CONV dynrec_sar_byte_simple(System::Byte op1,System::Byte op2) DRC_FC;
static System::Byte DRC_CALL_CONV dynrec_sar_byte_simple(System::Byte op1,System::Byte op2) {
	if (!op2) return op1;
	if (op2>8) op2=8;
    if (op1 & 0x80) return (op1 >> op2) | (0xff << (8 - op2));
	else return op1 >> op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_rol_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_rol_word(System::UInt16 op1,System::Byte op2) {
	if (!(op2&0xf)) {
		if (op2&0x10) {
			FillFlagsNoCFOF();
			SETFLAGBIT(CF,op1 & 1);
			SETFLAGBIT(OF,(op1 & 1) ^ (op1 >> 15));
		}
		return op1;
	}
	FillFlagsNoCFOF();
	lf_var1w=op1;
	lf_var2b=op2&0xf;
	lf_resw=(lf_var1w << lf_var2b) | (lf_var1w >> (16-lf_var2b));
	SETFLAGBIT(CF,lf_resw & 1);
	SETFLAGBIT(OF,(lf_resw & 1) ^ (lf_resw >> 15));
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_rol_word_simple(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_rol_word_simple(System::UInt16 op1,System::Byte op2) {
	if (!(op2&0xf)) return op1;
	return (op1 << (op2&0xf)) | (op1 >> (16-(op2&0xf)));
}

static System::UInt16 DRC_CALL_CONV dynrec_ror_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_ror_word(System::UInt16 op1,System::Byte op2) {
	if (!(op2&0xf)) {
		if (op2&0x10) {
			FillFlagsNoCFOF();
			SETFLAGBIT(CF,op1>>15);
			SETFLAGBIT(OF,(op1>>15) ^ ((op1>>14) & 1));
		}
		return op1;
	}
	FillFlagsNoCFOF();
	lf_var1w=op1;
	lf_var2b=op2&0xf;
	lf_resw=(lf_var1w >> lf_var2b) | (lf_var1w << (16-lf_var2b));
	SETFLAGBIT(CF,lf_resw & 0x8000);
	SETFLAGBIT(OF,(lf_resw ^ (lf_resw<<1)) & 0x8000);
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_ror_word_simple(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_ror_word_simple(System::UInt16 op1,System::Byte op2) {
	if (!(op2&0xf)) return op1;
	return (op1 >> (op2&0xf)) | (op1 << (16-(op2&0xf)));
}

static System::UInt16 DRC_CALL_CONV dynrec_rcl_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_rcl_word(System::UInt16 op1,System::Byte op2) {
	if (op2%17) {
		System::UInt16 cf=(System::UInt16)FillFlags()&0x1;
		lf_var1w=op1;
		lf_var2b=op2%17;
		lf_resw=(lf_var1w << lf_var2b) | (cf << (lf_var2b-1)) | (lf_var1w >> (17-lf_var2b));
		SETFLAGBIT(CF,((lf_var1w >> (16-lf_var2b)) & 1));
		SETFLAGBIT(OF,(reg_flags & 1) ^ (lf_resw >> 15));
		return lf_resw;
	} else return op1;
}

static System::UInt16 DRC_CALL_CONV dynrec_rcr_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_rcr_word(System::UInt16 op1,System::Byte op2) {
	if (op2%17) {
		System::UInt16 cf=(System::UInt16)FillFlags()&0x1;
		lf_var1w=op1;
		lf_var2b=op2%17;
	 	lf_resw=(lf_var1w >> lf_var2b) | (cf << (16-lf_var2b)) | (lf_var1w << (17-lf_var2b));
		SETFLAGBIT(CF,(lf_var1w >> (lf_var2b - 1)) & 1);
		SETFLAGBIT(OF,(lf_resw ^ (lf_resw<<1)) & 0x8000);
		return lf_resw;
	} else return op1;
}

static System::UInt16 DRC_CALL_CONV dynrec_shl_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_shl_word(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1w=op1;
	lf_var2b=op2;
	lf_resw=lf_var1w << lf_var2b;
	lflags.type=t_SHLw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_shl_word_simple(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_shl_word_simple(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 << op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_shr_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_shr_word(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1w=op1;
	lf_var2b=op2;
	lf_resw=lf_var1w >> lf_var2b;
	lflags.type=t_SHRw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_shr_word_simple(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_shr_word_simple(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 >> op2;
}

static System::UInt16 DRC_CALL_CONV dynrec_sar_word(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sar_word(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1w=op1;
	lf_var2b=op2;
	if (lf_var2b>16) lf_var2b=16;
	if (lf_var1w & 0x8000) {
		lf_resw=(lf_var1w >> lf_var2b) | (0xffff << (16 - lf_var2b));
	} else {
		lf_resw=lf_var1w >> lf_var2b;
    }
	lflags.type=t_SARw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_sar_word_simple(System::UInt16 op1,System::Byte op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_sar_word_simple(System::UInt16 op1,System::Byte op2) {
	if (!op2) return op1;
	if (op2>16) op2=16;
	if (op1 & 0x8000) return (op1 >> op2) | (0xffff << (16 - op2));
	else return op1 >> op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_rol_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_rol_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	FillFlagsNoCFOF();
	lf_var1d=op1;
	lf_var2b=op2;
	lf_resd=(lf_var1d << lf_var2b) | (lf_var1d >> (32-lf_var2b));
	SETFLAGBIT(CF,lf_resd & 1);
	SETFLAGBIT(OF,(lf_resd & 1) ^ (lf_resd >> 31));
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_rol_dword_simple(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_rol_dword_simple(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	return (op1 << op2) | (op1 >> (32-op2));
}

static System::UInt32 DRC_CALL_CONV dynrec_ror_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_ror_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	FillFlagsNoCFOF();
	lf_var1d=op1;
	lf_var2b=op2;
	lf_resd=(lf_var1d >> lf_var2b) | (lf_var1d << (32-lf_var2b));
	SETFLAGBIT(CF,lf_resd & 0x80000000);
	SETFLAGBIT(OF,(lf_resd ^ (lf_resd<<1)) & 0x80000000);
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_ror_dword_simple(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_ror_dword_simple(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	return (op1 >> op2) | (op1 << (32-op2));
}

static System::UInt32 DRC_CALL_CONV dynrec_rcl_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_rcl_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	System::UInt32 cf=(System::UInt32)FillFlags()&0x1;
	lf_var1d=op1;
	lf_var2b=op2;
	if (lf_var2b==1) {
		lf_resd=(lf_var1d << 1) | cf;
	} else 	{
		lf_resd=(lf_var1d << lf_var2b) | (cf << (lf_var2b-1)) | (lf_var1d >> (33-lf_var2b));
	}
	SETFLAGBIT(CF,((lf_var1d >> (32-lf_var2b)) & 1));
	SETFLAGBIT(OF,(reg_flags & 1) ^ (lf_resd >> 31));
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_rcr_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_rcr_dword(System::UInt32 op1,System::Byte op2) {
	if (op2) {
		System::UInt32 cf=(System::UInt32)FillFlags()&0x1;
		lf_var1d=op1;
		lf_var2b=op2;
		if (lf_var2b==1) {
			lf_resd=lf_var1d >> 1 | cf << 31;
		} else {
 			lf_resd=(lf_var1d >> lf_var2b) | (cf << (32-lf_var2b)) | (lf_var1d << (33-lf_var2b));
		}
		SETFLAGBIT(CF,(lf_var1d >> (lf_var2b - 1)) & 1);
		SETFLAGBIT(OF,(lf_resd ^ (lf_resd<<1)) & 0x80000000);
		return lf_resd;
	} else return op1;
}

static System::UInt32 DRC_CALL_CONV dynrec_shl_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_shl_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1d=op1;
	lf_var2b=op2;
	lf_resd=lf_var1d << lf_var2b;
	lflags.type=t_SHLd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_shl_dword_simple(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_shl_dword_simple(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 << op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_shr_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_shr_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var1d=op1;
	lf_var2b=op2;
	lf_resd=lf_var1d >> lf_var2b;
	lflags.type=t_SHRd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_shr_dword_simple(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_shr_dword_simple(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	return op1 >> op2;
}

static System::UInt32 DRC_CALL_CONV dynrec_sar_dword(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sar_dword(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	lf_var2b=op2;
	lf_var1d=op1;
	if (lf_var1d & 0x80000000) {
		lf_resd=(lf_var1d >> lf_var2b) | (0xffffffff << (32 - lf_var2b));
	} else {
		lf_resd=lf_var1d >> lf_var2b;
    }
	lflags.type=t_SARd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_sar_dword_simple(System::UInt32 op1,System::Byte op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_sar_dword_simple(System::UInt32 op1,System::Byte op2) {
	if (!op2) return op1;
	if (op1 & 0x80000000) return (op1 >> op2) | (0xffffffff << (32 - op2));
	else return op1 >> op2;
}

static void dyn_shift_byte_gencall(ShiftOps op) {
	switch (op) {
		case SHIFT_ROL:
			InvalidateFlagsPartially((void*)&dynrec_rol_byte_simple,t_ROLb);
			gen_call_function_raw((void*)&dynrec_rol_byte);
			break;
		case SHIFT_ROR:
			InvalidateFlagsPartially((void*)&dynrec_ror_byte_simple,t_RORb);
			gen_call_function_raw((void*)&dynrec_ror_byte);
			break;
		case SHIFT_RCL:
			AcquireFlags(FLAG_CF);
			gen_call_function_raw((void*)&dynrec_rcl_byte);
			break;
		case SHIFT_RCR:
			AcquireFlags(FLAG_CF);
			gen_call_function_raw((void*)&dynrec_rcr_byte);
			break;
		case SHIFT_SHL:
		case SHIFT_SAL:
			InvalidateFlagsPartially((void*)&dynrec_shl_byte_simple,t_SHLb);
			gen_call_function_raw((void*)&dynrec_shl_byte);
			break;
		case SHIFT_SHR:
			InvalidateFlagsPartially((void*)&dynrec_shr_byte_simple,t_SHRb);
			gen_call_function_raw((void*)&dynrec_shr_byte);
			break;
		case SHIFT_SAR:
			InvalidateFlagsPartially((void*)&dynrec_sar_byte_simple,t_SARb);
			gen_call_function_raw((void*)&dynrec_sar_byte);
			break;
		default: IllegalOptionDynrec("dyn_shift_byte_gencall");
	}
}

static void dyn_shift_word_gencall(ShiftOps op,bool dword) {
	if (dword) {
		switch (op) {
			case SHIFT_ROL:
				InvalidateFlagsPartially((void*)&dynrec_rol_dword_simple,t_ROLd);
				gen_call_function_raw((void*)&dynrec_rol_dword);
				break;
			case SHIFT_ROR:
				InvalidateFlagsPartially((void*)&dynrec_ror_dword_simple,t_RORd);
				gen_call_function_raw((void*)&dynrec_ror_dword);
				break;
			case SHIFT_RCL:
				AcquireFlags(FLAG_CF);
				gen_call_function_raw((void*)&dynrec_rcl_dword);
				break;
			case SHIFT_RCR:
				AcquireFlags(FLAG_CF);
				gen_call_function_raw((void*)&dynrec_rcr_dword);
				break;
			case SHIFT_SHL:
			case SHIFT_SAL:
				InvalidateFlagsPartially((void*)&dynrec_shl_dword_simple,t_SHLd);
				gen_call_function_raw((void*)&dynrec_shl_dword);
				break;
			case SHIFT_SHR:
				InvalidateFlagsPartially((void*)&dynrec_shr_dword_simple,t_SHRd);
				gen_call_function_raw((void*)&dynrec_shr_dword);
				break;
			case SHIFT_SAR:
				InvalidateFlagsPartially((void*)&dynrec_sar_dword_simple,t_SARd);
				gen_call_function_raw((void*)&dynrec_sar_dword);
				break;
			default: IllegalOptionDynrec("dyn_shift_dword_gencall");
		}
	} else {
		switch (op) {
			case SHIFT_ROL:
				InvalidateFlagsPartially((void*)&dynrec_rol_word_simple,t_ROLw);
				gen_call_function_raw((void*)&dynrec_rol_word);
				break;
			case SHIFT_ROR:
				InvalidateFlagsPartially((void*)&dynrec_ror_word_simple,t_RORw);
				gen_call_function_raw((void*)&dynrec_ror_word);
				break;
			case SHIFT_RCL:
				AcquireFlags(FLAG_CF);
				gen_call_function_raw((void*)&dynrec_rcl_word);
				break;
			case SHIFT_RCR:
				AcquireFlags(FLAG_CF);
				gen_call_function_raw((void*)&dynrec_rcr_word);
				break;
			case SHIFT_SHL:
			case SHIFT_SAL:
				InvalidateFlagsPartially((void*)&dynrec_shl_word_simple,t_SHLw);
				gen_call_function_raw((void*)&dynrec_shl_word);
				break;
			case SHIFT_SHR:
				InvalidateFlagsPartially((void*)&dynrec_shr_word_simple,t_SHRw);
				gen_call_function_raw((void*)&dynrec_shr_word);
				break;
			case SHIFT_SAR:
				InvalidateFlagsPartially((void*)&dynrec_sar_word_simple,t_SARw);
				gen_call_function_raw((void*)&dynrec_sar_word);
				break;
			default: IllegalOptionDynrec("dyn_shift_word_gencall");
		}
	}
}

static System::UInt16 DRC_CALL_CONV dynrec_dshl_word(System::UInt16 op1,System::UInt16 op2,System::Byte op3) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dshl_word(System::UInt16 op1,System::UInt16 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	lf_var2b=val;
	lf_var1d=(op1<<16)|op2;
	System::UInt32 tempd=lf_var1d << lf_var2b;
  	if (lf_var2b>16) tempd |= (op2 << (lf_var2b - 16));
	lf_resw=(System::UInt16)(tempd >> 16);
	lflags.type=t_DSHLw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_dshl_word_simple(System::UInt16 op1,System::UInt16 op2,System::Byte op3) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dshl_word_simple(System::UInt16 op1,System::UInt16 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	System::UInt32 tempd=(System::UInt32)((((System::UInt32)op1)<<16)|op2) << val;
  	if (val>16) tempd |= (op2 << (val - 16));
	return (System::UInt16)(tempd >> 16);
}

static System::UInt32 DRC_CALL_CONV dynrec_dshl_dword(System::UInt32 op1,System::UInt32 op2,System::Byte op3) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dshl_dword(System::UInt32 op1,System::UInt32 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	lf_var2b=val;
	lf_var1d=op1;
	lf_resd=(lf_var1d << lf_var2b) | (op2 >> (32-lf_var2b));
	lflags.type=t_DSHLd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_dshl_dword_simple(System::UInt32 op1,System::UInt32 op2,System::Byte op3) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dshl_dword_simple(System::UInt32 op1,System::UInt32 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	return (op1 << val) | (op2 >> (32-val));
}

static System::UInt16 DRC_CALL_CONV dynrec_dshr_word(System::UInt16 op1,System::UInt16 op2,System::Byte op3) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dshr_word(System::UInt16 op1,System::UInt16 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	lf_var2b=val;
	lf_var1d=(op2<<16)|op1;
	System::UInt32 tempd=lf_var1d >> lf_var2b;
  	if (lf_var2b>16) tempd |= (op2 << (32-lf_var2b ));
	lf_resw=(System::UInt16)(tempd);
	lflags.type=t_DSHRw;
	return lf_resw;
}

static System::UInt16 DRC_CALL_CONV dynrec_dshr_word_simple(System::UInt16 op1,System::UInt16 op2,System::Byte op3) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dshr_word_simple(System::UInt16 op1,System::UInt16 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	System::UInt32 tempd=(System::UInt32)((((System::UInt32)op2)<<16)|op1) >> val;
  	if (val>16) tempd |= (op2 << (32-val));
	return (System::UInt16)(tempd);
}

static System::UInt32 DRC_CALL_CONV dynrec_dshr_dword(System::UInt32 op1,System::UInt32 op2,System::Byte op3) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dshr_dword(System::UInt32 op1,System::UInt32 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	lf_var2b=val;
	lf_var1d=op1;
	lf_resd=(lf_var1d >> lf_var2b) | (op2 << (32-lf_var2b));
	lflags.type=t_DSHRd;
	return lf_resd;
}

static System::UInt32 DRC_CALL_CONV dynrec_dshr_dword_simple(System::UInt32 op1,System::UInt32 op2,System::Byte op3) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dshr_dword_simple(System::UInt32 op1,System::UInt32 op2,System::Byte op3) {
	System::Byte val=op3 & 0x1f;
	if (!val) return op1;
	return (op1 >> val) | (op2 << (32-val));
}

static void dyn_dpshift_word_gencall(bool left) {
	if (left) {
		DRC_PTR_SIZE_IM proc_addr=gen_call_function_R3((void*)&dynrec_dshl_word,FC_OP3);
		InvalidateFlagsPartially((void*)&dynrec_dshl_word_simple,proc_addr,t_DSHLw);
	} else {
		DRC_PTR_SIZE_IM proc_addr=gen_call_function_R3((void*)&dynrec_dshr_word,FC_OP3);
		InvalidateFlagsPartially((void*)&dynrec_dshr_word_simple,proc_addr,t_DSHRw);
	}
}

static void dyn_dpshift_dword_gencall(bool left) {
	if (left) {
		DRC_PTR_SIZE_IM proc_addr=gen_call_function_R3((void*)&dynrec_dshl_dword,FC_OP3);
		InvalidateFlagsPartially((void*)&dynrec_dshl_dword_simple,proc_addr,t_DSHLd);
	} else {
		DRC_PTR_SIZE_IM proc_addr=gen_call_function_R3((void*)&dynrec_dshr_dword,FC_OP3);
		InvalidateFlagsPartially((void*)&dynrec_dshr_dword_simple,proc_addr,t_DSHRd);
	}
}



static System::UInt32 DRC_CALL_CONV dynrec_get_of(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_of(void)		{ return TFLG_O; }
static System::UInt32 DRC_CALL_CONV dynrec_get_nof(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_nof(void)	{ return TFLG_NO; }
static System::UInt32 DRC_CALL_CONV dynrec_get_cf(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_cf(void)		{ return TFLG_B; }
static System::UInt32 DRC_CALL_CONV dynrec_get_ncf(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_ncf(void)	{ return TFLG_NB; }
static System::UInt32 DRC_CALL_CONV dynrec_get_zf(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_zf(void)		{ return TFLG_Z; }
static System::UInt32 DRC_CALL_CONV dynrec_get_nzf(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_nzf(void)	{ return TFLG_NZ; }
static System::UInt32 DRC_CALL_CONV dynrec_get_sf(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_sf(void)		{ return TFLG_S; }
static System::UInt32 DRC_CALL_CONV dynrec_get_nsf(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_nsf(void)	{ return TFLG_NS; }
static System::UInt32 DRC_CALL_CONV dynrec_get_pf(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_pf(void)		{ return TFLG_P; }
static System::UInt32 DRC_CALL_CONV dynrec_get_npf(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_npf(void)	{ return TFLG_NP; }

static System::UInt32 DRC_CALL_CONV dynrec_get_cf_or_zf(void)			DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_cf_or_zf(void)			{ return TFLG_BE; }
static System::UInt32 DRC_CALL_CONV dynrec_get_ncf_and_nzf(void)		DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_ncf_and_nzf(void)		{ return TFLG_NBE; }
static System::UInt32 DRC_CALL_CONV dynrec_get_sf_neq_of(void)			DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_sf_neq_of(void)			{ return TFLG_L; }
static System::UInt32 DRC_CALL_CONV dynrec_get_sf_eq_of(void)			DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_sf_eq_of(void)			{ return TFLG_NL; }
static System::UInt32 DRC_CALL_CONV dynrec_get_zf_or_sf_neq_of(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_zf_or_sf_neq_of(void)	{ return TFLG_LE; }
static System::UInt32 DRC_CALL_CONV dynrec_get_nzf_and_sf_eq_of(void)	DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_get_nzf_and_sf_eq_of(void)	{ return TFLG_NLE; }


static void dyn_branchflag_to_reg(BranchTypes btype) {
	switch (btype) {
		case BR_O:gen_call_function_raw((void*)&dynrec_get_of);break;
		case BR_NO:gen_call_function_raw((void*)&dynrec_get_nof);break;
		case BR_B:gen_call_function_raw((void*)&dynrec_get_cf);break;
		case BR_NB:gen_call_function_raw((void*)&dynrec_get_ncf);break;
		case BR_Z:gen_call_function_raw((void*)&dynrec_get_zf);break;
		case BR_NZ:gen_call_function_raw((void*)&dynrec_get_nzf);break;
		case BR_BE:gen_call_function_raw((void*)&dynrec_get_cf_or_zf);break;
		case BR_NBE:gen_call_function_raw((void*)&dynrec_get_ncf_and_nzf);break;

		case BR_S:gen_call_function_raw((void*)&dynrec_get_sf);break;
		case BR_NS:gen_call_function_raw((void*)&dynrec_get_nsf);break;
		case BR_P:gen_call_function_raw((void*)&dynrec_get_pf);break;
		case BR_NP:gen_call_function_raw((void*)&dynrec_get_npf);break;
		case BR_L:gen_call_function_raw((void*)&dynrec_get_sf_neq_of);break;
		case BR_NL:gen_call_function_raw((void*)&dynrec_get_sf_eq_of);break;
		case BR_LE:gen_call_function_raw((void*)&dynrec_get_zf_or_sf_neq_of);break;
		case BR_NLE:gen_call_function_raw((void*)&dynrec_get_nzf_and_sf_eq_of);break;
	}
}


static void DRC_CALL_CONV dynrec_mul_byte(System::Byte op) DRC_FC;
static void DRC_CALL_CONV dynrec_mul_byte(System::Byte op) {
	FillFlagsNoCFOF();
	reg_ax=reg_al*op;
	SETFLAGBIT(ZF,reg_al == 0);
	if (reg_ax & 0xff00) {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	} else {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	}
}

static void DRC_CALL_CONV dynrec_imul_byte(System::Byte op) DRC_FC;
static void DRC_CALL_CONV dynrec_imul_byte(System::Byte op) {
	FillFlagsNoCFOF();
	reg_ax=((System::SByte)reg_al) * ((System::SByte)op);
	if ((reg_ax & 0xff80)==0xff80 || (reg_ax & 0xff80)==0x0000) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	}
}

static void DRC_CALL_CONV dynrec_mul_word(System::UInt16 op) DRC_FC;
static void DRC_CALL_CONV dynrec_mul_word(System::UInt16 op) {
	FillFlagsNoCFOF();
	unsigned tempu=(unsigned)reg_ax*(unsigned)op;
	reg_ax=(System::UInt16)(tempu);
	reg_dx=(System::UInt16)(tempu >> 16);
	SETFLAGBIT(ZF,reg_ax == 0);
	if (reg_dx) {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	} else {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	}
}

static void DRC_CALL_CONV dynrec_imul_word(System::UInt16 op) DRC_FC;
static void DRC_CALL_CONV dynrec_imul_word(System::UInt16 op) {
	FillFlagsNoCFOF();
	int temps=((System::Int16)reg_ax)*((System::Int16)op);
	reg_ax=(System::Int16)(temps);
	reg_dx=(System::Int16)(temps >> 16);
	if (((temps & 0xffff8000)==0xffff8000 || (temps & 0xffff8000)==0x0000)) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	}
}

static void DRC_CALL_CONV dynrec_mul_dword(System::UInt32 op) DRC_FC;
static void DRC_CALL_CONV dynrec_mul_dword(System::UInt32 op) {
	FillFlagsNoCFOF();
	System::UInt64 tempu=(System::UInt64)reg_eax*(System::UInt64)op;
	reg_eax=(System::UInt32)(tempu);
	reg_edx=(System::UInt32)(tempu >> 32);
	SETFLAGBIT(ZF,reg_eax == 0);
	if (reg_edx) {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	} else {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	}
}

static void DRC_CALL_CONV dynrec_imul_dword(System::UInt32 op) DRC_FC;
static void DRC_CALL_CONV dynrec_imul_dword(System::UInt32 op) {
	FillFlagsNoCFOF();
	System::Int64 temps=((System::Int64)((System::Int32)reg_eax))*((System::Int64)((System::Int32)op));
	reg_eax=(System::UInt32)(temps);
	reg_edx=(System::UInt32)(temps >> 32);
	if ((reg_edx==0xffffffff) && (reg_eax & 0x80000000) ) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else if ( (reg_edx==0x00000000) && (reg_eax< 0x80000000) ) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	}
}


static bool DRC_CALL_CONV dynrec_div_byte(System::Byte op) DRC_FC;
static bool DRC_CALL_CONV dynrec_div_byte(System::Byte op) {
	unsigned val=op;
	if (val==0) return CPU_PrepareException(0,0);
	unsigned quo=reg_ax / val;
	System::Byte rem=(System::Byte)(reg_ax % val);
	System::Byte quo8=(System::Byte)(quo&0xff);
	if (quo>0xff) return CPU_PrepareException(0,0);
	reg_ah=rem;
	reg_al=quo8;
	return false;
}

static bool DRC_CALL_CONV dynrec_idiv_byte(System::Byte op) DRC_FC;
static bool DRC_CALL_CONV dynrec_idiv_byte(System::Byte op) {
	int val=(System::SByte)op;
	if (val==0) return CPU_PrepareException(0,0);
	int quo=((System::Int16)reg_ax) / val;
	System::SByte rem=(System::SByte)((System::Int16)reg_ax % val);
	System::SByte quo8s=(System::SByte)(quo&0xff);
	if (quo!=(System::Int16)quo8s) return CPU_PrepareException(0,0);
	reg_ah=rem;
	reg_al=quo8s;
	return false;
}

static bool DRC_CALL_CONV dynrec_div_word(System::UInt16 op) DRC_FC;
static bool DRC_CALL_CONV dynrec_div_word(System::UInt16 op) {
	unsigned val=op;
	if (val==0)	return CPU_PrepareException(0,0);
	unsigned num=((System::UInt32)reg_dx<<16)|reg_ax;
	unsigned quo=num/val;
	System::UInt16 rem=(System::UInt16)(num % val);
	System::UInt16 quo16=(System::UInt16)(quo&0xffff);
	if (quo!=(System::UInt32)quo16) return CPU_PrepareException(0,0);
	reg_dx=rem;
	reg_ax=quo16;
	return false;
}

static bool DRC_CALL_CONV dynrec_idiv_word(System::UInt16 op) DRC_FC;
static bool DRC_CALL_CONV dynrec_idiv_word(System::UInt16 op) {
	int val=(System::Int16)op;
	if (val==0) return CPU_PrepareException(0,0);
	int num=(System::Int32)((reg_dx<<16)|reg_ax);
	int quo=num/val;
	System::Int16 rem=(System::Int16)(num % val);
	System::Int16 quo16s=(System::Int16)quo;
	if (quo!=(System::Int32)quo16s) return CPU_PrepareException(0,0);
	reg_dx=rem;
	reg_ax=quo16s;
	return false;
}

static bool DRC_CALL_CONV dynrec_div_dword(System::UInt32 op) DRC_FC;
static bool DRC_CALL_CONV dynrec_div_dword(System::UInt32 op) {
	unsigned val=op;
	if (val==0) return CPU_PrepareException(0,0);
	System::UInt64 num=(((System::UInt64)reg_edx)<<32)|reg_eax;
	System::UInt64 quo=num/val;
	System::UInt32 rem=(System::UInt32)(num % val);
	System::UInt32 quo32=(System::UInt32)(quo&0xffffffff);
	if (quo!=(System::UInt64)quo32) return CPU_PrepareException(0,0);
	reg_edx=rem;
	reg_eax=quo32;
	return false;
}

static bool DRC_CALL_CONV dynrec_idiv_dword(System::UInt32 op) DRC_FC;
static bool DRC_CALL_CONV dynrec_idiv_dword(System::UInt32 op) {
	int val=(System::Int32)op;
	if (val==0) return CPU_PrepareException(0,0);
	System::Int64 num=(((System::UInt64)reg_edx)<<32)|reg_eax;	
	System::Int64 quo=num/val;
	System::Int32 rem=(System::Int32)(num % val);
	System::Int32 quo32s=(System::Int32)(quo&0xffffffff);
	if (quo!=(System::Int64)quo32s) return CPU_PrepareException(0,0);
	reg_edx=rem;
	reg_eax=quo32s;
	return false;
}


static System::UInt16 DRC_CALL_CONV dynrec_dimul_word(System::UInt16 op1,System::UInt16 op2) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_dimul_word(System::UInt16 op1,System::UInt16 op2) {
	FillFlagsNoCFOF();
	int res=((System::Int16)op1) * ((System::Int16)op2);
	if ((res>-32768)  && (res<32767)) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	}
	return (System::UInt16)(res & 0xffff);
}

static System::UInt32 DRC_CALL_CONV dynrec_dimul_dword(System::UInt32 op1,System::UInt32 op2) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_dimul_dword(System::UInt32 op1,System::UInt32 op2) {
	FillFlagsNoCFOF();
	System::Int64 res=((System::Int64)((System::Int32)op1))*((System::Int64)((System::Int32)op2));
	if ((res>-((System::Int64)(2147483647)+1)) && (res<(System::Int64)2147483647)) {
		SETFLAGBIT(CF,false);
		SETFLAGBIT(OF,false);
	} else {
		SETFLAGBIT(CF,true);
		SETFLAGBIT(OF,true);
	}
	return (System::Int32)res;
}



static System::UInt16 DRC_CALL_CONV dynrec_cbw(System::Byte op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_cbw(System::Byte op) {
	return (System::SByte)op;
}

static System::UInt32 DRC_CALL_CONV dynrec_cwde(System::UInt16 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_cwde(System::UInt16 op) {
	return (System::Int16)op;
}

static System::UInt16 DRC_CALL_CONV dynrec_cwd(System::UInt16 op) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_cwd(System::UInt16 op) {
	if (op & 0x8000) return 0xffff;
	else return 0;
}

static System::UInt32 DRC_CALL_CONV dynrec_cdq(System::UInt32 op) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_cdq(System::UInt32 op) {
	if (op & 0x80000000) return 0xffffffff;
	else return 0;
}


static void DRC_CALL_CONV dynrec_sahf(System::UInt16 op) DRC_FC;
static void DRC_CALL_CONV dynrec_sahf(System::UInt16 op) {
	SETFLAGBIT(OF,get_OF());
	lflags.type=t_UNKNOWN;
	CPU_SetFlags(op>>8,FMASK_NORMAL & 0xff);
}


static void DRC_CALL_CONV dynrec_cmc(void) DRC_FC;
static void DRC_CALL_CONV dynrec_cmc(void) {
	FillFlags();
	SETFLAGBIT(CF,!(reg_flags & FLAG_CF));
}
static void DRC_CALL_CONV dynrec_clc(void) DRC_FC;
static void DRC_CALL_CONV dynrec_clc(void) {
	FillFlags();
	SETFLAGBIT(CF,false);
}
static void DRC_CALL_CONV dynrec_stc(void) DRC_FC;
static void DRC_CALL_CONV dynrec_stc(void) {
	FillFlags();
	SETFLAGBIT(CF,true);
}
static void DRC_CALL_CONV dynrec_cld(void) DRC_FC;
static void DRC_CALL_CONV dynrec_cld(void) {
	SETFLAGBIT(DF,false);
	cpu.direction=1;
}
static void DRC_CALL_CONV dynrec_std(void) DRC_FC;
static void DRC_CALL_CONV dynrec_std(void) {
	SETFLAGBIT(DF,true);
	cpu.direction=-1;
}


static System::UInt16 DRC_CALL_CONV dynrec_movsb_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_movsb_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		mem_writeb(di_base+reg_di,mem_readb(si_base+reg_si));
		reg_si+=add_index;
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_movsb_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_movsb_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		mem_writeb(di_base+reg_edi,mem_readb(si_base+reg_esi));
		reg_esi+=add_index;
		reg_edi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_movsw_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_movsw_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		mem_writew(di_base+reg_di,mem_readw(si_base+reg_si));
		reg_si+=add_index;
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_movsw_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_movsw_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		mem_writew(di_base+reg_edi,mem_readw(si_base+reg_esi));
		reg_esi+=add_index;
		reg_edi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_movsd_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_movsd_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		mem_writed(di_base+reg_di,mem_readd(si_base+reg_si));
		reg_si+=add_index;
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_movsd_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_movsd_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		mem_writed(di_base+reg_edi,mem_readd(si_base+reg_esi));
		reg_esi+=add_index;
		reg_edi+=add_index;
	}
	return count_left;
}


static System::UInt16 DRC_CALL_CONV dynrec_lodsb_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_lodsb_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		reg_al=mem_readb(si_base+reg_si);
		reg_si+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_lodsb_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_lodsb_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		reg_al=mem_readb(si_base+reg_esi);
		reg_esi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_lodsw_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_lodsw_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		reg_ax=mem_readw(si_base+reg_si);
		reg_si+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_lodsw_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_lodsw_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		reg_ax=mem_readw(si_base+reg_esi);
		reg_esi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_lodsd_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_lodsd_word(System::UInt16 count,System::Int16 add_index,PhysPt si_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		reg_eax=mem_readd(si_base+reg_si);
		reg_si+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_lodsd_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_lodsd_dword(System::UInt32 count,System::Int32 add_index,PhysPt si_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		reg_eax=mem_readd(si_base+reg_esi);
		reg_esi+=add_index;
	}
	return count_left;
}


static System::UInt16 DRC_CALL_CONV dynrec_stosb_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_stosb_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		mem_writeb(di_base+reg_di,reg_al);
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_stosb_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_stosb_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	for (;count>0;count--) {
		mem_writeb(di_base+reg_edi,reg_al);
		reg_edi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_stosw_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_stosw_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		mem_writew(di_base+reg_di,reg_ax);
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_stosw_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_stosw_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=1;
	for (;count>0;count--) {
		mem_writew(di_base+reg_edi,reg_ax);
		reg_edi+=add_index;
	}
	return count_left;
}

static System::UInt16 DRC_CALL_CONV dynrec_stosd_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_stosd_word(System::UInt16 count,System::Int16 add_index,PhysPt di_base) {
	System::UInt16 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=(System::UInt16)(count-CPU_Cycles);
		count=(System::UInt16)CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		mem_writed(di_base+reg_di,reg_eax);
		reg_di+=add_index;
	}
	return count_left;
}

static System::UInt32 DRC_CALL_CONV dynrec_stosd_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_stosd_dword(System::UInt32 count,System::Int32 add_index,PhysPt di_base) {
	System::UInt32 count_left;
	if (count<(unsigned)CPU_Cycles) {
		count_left=0;
	} else {
		count_left=count-CPU_Cycles;
		count=CPU_Cycles;
		CPU_Cycles=0;
	}
	add_index<<=2;
	for (;count>0;count--) {
		mem_writed(di_base+reg_edi,reg_eax);
		reg_edi+=add_index;
	}
	return count_left;
}


static void DRC_CALL_CONV dynrec_push_word(System::UInt16 value) DRC_FC;
static void DRC_CALL_CONV dynrec_push_word(System::UInt16 value) {
	System::UInt32 new_esp=(reg_esp&cpu.stack.notmask)|((reg_esp-2)&cpu.stack.mask);
	mem_writew(SegPhys(ss) + (new_esp & cpu.stack.mask),value);
	reg_esp=new_esp;
}

static void DRC_CALL_CONV dynrec_push_dword(System::UInt32 value) DRC_FC;
static void DRC_CALL_CONV dynrec_push_dword(System::UInt32 value) {
	System::UInt32 new_esp=(reg_esp&cpu.stack.notmask)|((reg_esp-4)&cpu.stack.mask);
	mem_writed(SegPhys(ss) + (new_esp & cpu.stack.mask) ,value);
	reg_esp=new_esp;
}

static System::UInt16 DRC_CALL_CONV dynrec_pop_word(void) DRC_FC;
static System::UInt16 DRC_CALL_CONV dynrec_pop_word(void) {
	System::UInt16 val=mem_readw(SegPhys(ss) + (reg_esp & cpu.stack.mask));
	reg_esp=(reg_esp&cpu.stack.notmask)|((reg_esp+2)&cpu.stack.mask);
	return val;
}

static System::UInt32 DRC_CALL_CONV dynrec_pop_dword(void) DRC_FC;
static System::UInt32 DRC_CALL_CONV dynrec_pop_dword(void) {
	System::UInt32 val=mem_readd(SegPhys(ss) + (reg_esp & cpu.stack.mask));
	reg_esp=(reg_esp&cpu.stack.notmask)|((reg_esp+4)&cpu.stack.mask);
	return val;
}
