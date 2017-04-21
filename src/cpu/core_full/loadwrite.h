#define SaveIP() reg_eip=(System::UInt32)(inst.cseip-SegBase(cs));
#define LoadIP() inst.cseip=SegBase(cs)+reg_eip;
#define GetIP()	(inst.cseip-SegBase(cs))

#define RunException() {										\
	CPU_Exception(cpu.exception.which,cpu.exception.error);		\
	continue;													\
}

static INLINE System::Byte the_Fetchb(EAPoint & loc) {
	System::Byte temp=LoadMb(loc);
	loc+=1;
	return temp;
}
	
static INLINE System::UInt16 the_Fetchw(EAPoint & loc) {
	System::UInt16 temp=LoadMw(loc);
	loc+=2;
	return temp;
}
static INLINE System::UInt32 the_Fetchd(EAPoint & loc) {
	System::UInt32 temp=LoadMd(loc);
	loc+=4;
	return temp;
}

#define Fetchb() the_Fetchb(inst.cseip)
#define Fetchw() the_Fetchw(inst.cseip)
#define Fetchd() the_Fetchd(inst.cseip)

#define Fetchbs() (System::SByte)the_Fetchb(inst.cseip)
#define Fetchws() (System::Int16)the_Fetchw(inst.cseip)
#define Fetchds() (System::Int32)the_Fetchd(inst.cseip)

#define Push_16 CPU_Push16
#define Push_32 CPU_Push32
#define Pop_16 CPU_Pop16
#define Pop_32 CPU_Pop32

