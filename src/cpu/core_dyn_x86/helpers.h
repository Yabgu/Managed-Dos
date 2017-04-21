static bool dyn_helper_divb(System::Byte val) {
	if (!val) return CPU_PrepareException(0,0);
	unsigned quo=reg_ax / val;
	System::Byte rem=(System::Byte)(reg_ax % val);
	System::Byte quo8=(System::Byte)(quo&0xff);
	if (quo>0xff) return CPU_PrepareException(0,0);
	reg_ah=rem;
	reg_al=quo8;
	return false;
}

static bool dyn_helper_idivb(System::SByte val) {
	if (!val) return CPU_PrepareException(0,0);
	int quo=(System::Int16)reg_ax / val;
	System::SByte rem=(System::SByte)((System::Int16)reg_ax % val);
	System::SByte quo8s=(System::SByte)(quo&0xff);
	if (quo!=(System::Int16)quo8s) return CPU_PrepareException(0,0);
	reg_ah=rem;
	reg_al=quo8s;
	return false;
}

static bool dyn_helper_divw(System::UInt16 val) {
	if (!val) return CPU_PrepareException(0,0);
	unsigned num=(reg_dx<<16)|reg_ax;
	unsigned quo=num/val;
	System::UInt16 rem=(System::UInt16)(num % val);
	System::UInt16 quo16=(System::UInt16)(quo&0xffff);
	if (quo!=(System::UInt32)quo16) return CPU_PrepareException(0,0);
	reg_dx=rem;
	reg_ax=quo16;
	return false;
}

static bool dyn_helper_idivw(System::Int16 val) {
	if (!val) return CPU_PrepareException(0,0);
	int num=(reg_dx<<16)|reg_ax;
	int quo=num/val;
	System::Int16 rem=(System::Int16)(num % val);
	System::Int16 quo16s=(System::Int16)quo;
	if (quo!=(System::Int32)quo16s) return CPU_PrepareException(0,0);
	reg_dx=rem;
	reg_ax=quo16s;
	return false;
}

static bool dyn_helper_divd(System::UInt32 val) {
	if (!val) return CPU_PrepareException(0,0);
	System::UInt64 num=(((System::UInt64)reg_edx)<<32)|reg_eax;
	System::UInt64 quo=num/val;
	System::UInt32 rem=(System::UInt32)(num % val);
	System::UInt32 quo32=(System::UInt32)(quo&0xffffffff);
	if (quo!=(System::UInt64)quo32) return CPU_PrepareException(0,0);
	reg_edx=rem;
	reg_eax=quo32;
	return false;
}

static bool dyn_helper_idivd(System::Int32 val) {
	if (!val) return CPU_PrepareException(0,0);
	System::Int64 num=(((System::UInt64)reg_edx)<<32)|reg_eax;
	System::Int64 quo=num/val;
	System::Int32 rem=(System::Int32)(num % val);
	System::Int32 quo32s=(System::Int32)(quo&0xffffffff);
	if (quo!=(System::Int64)quo32s) return CPU_PrepareException(0,0);
	reg_edx=rem;
	reg_eax=quo32s;
	return false;
}
