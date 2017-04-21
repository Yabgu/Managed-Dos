/* Standard data types used */

typedef unsigned char System::Byte;
typedef signed char System::SByte;
typedef unsigned short System::UInt16;
typedef signed short System::Int16;
typedef unsigned long System::UInt32;
typedef signed long System::Int32;
#if defined(_MSC_VER)
typedef unsigned __int64 System::UInt64;
typedef signed __int64 System::Int64;
#else
typedef unsigned long long int System::UInt64;
typedef signed long long int System::Int64;
#endif



/* Setting up pointers to all subfunctions */
#ifdef MODULE_WANT_IO_READ
typedef System::Byte (* IO_ReadHandler)(System::UInt32 port);
static void (* IO_RegisterReadHandler)(System::UInt32 port,IO_ReadHandler handler,char * name);
static void (* IO_FreeReadHandler)(System::UInt32 port);
#endif

#ifdef MODULE_WANT_IO_WRITE
typedef void (* IO_WriteHandler)(System::UInt32 port,System::Byte value);
static void (* IO_RegisterWriteHandler)(System::UInt32 port,IO_WriteHandler handler,char * name);
static void (* IO_FreeWriteHandler)(System::UInt32 port);
#endif

#ifdef MODULE_WANT_IRQ_EOI
typedef void (* IRQ_EOIHandler)(void);
static void (* IRQ_RegisterEOIHandler)(System::UInt32 irq,IRQ_EOIHandler handler,char * name);
static void (* IRQ_FreeEOIHandler)(System::UInt32 irq);
#endif

#ifdef MODULE_WANT_IRQ 
static void (* IRQ_Activate)(System::UInt32 irq);
static void (* IRQ_Deactivate)(System::UInt32 irq);
#endif

#ifdef MODULE_WANT_TIMER
typedef void (* TIMER_MicroHandler)(void);
static  void (* TIMER_RegisterMicroHandler)(TIMER_MicroHandler handler,System::UInt32 micro);
#endif

#ifdef MODULE_WANT_TIMER_TICK
typedef void (* TIMER_TickHandler)(System::UInt32 ticks);
static  void (* TIMER_RegisterTickHandler)(TIMER_TickHandler handler);
#endif

/* 
	4 8-bit and 4 16-bit channels you can read data from 
	16-bit reads are word sized
*/

#ifdef MODULE_WANT_DMA_READ
static void (* DMA_8_Read)(System::UInt32 chan,System::Byte * data,System::UInt16 size);
static void (* DMA_16_Read)(System::UInt32 chan,System::Byte * data,System::UInt16 size);
#endif

/* 
	4 8-bit and 4 16-bit channels you can write data from 
	16-bit writes are word sized
*/

#ifdef MODULE_WANT_DMA_READ
static void (* DMA_8_Write)(System::UInt32 chan,System::Byte * data,System::UInt16 size);
static void (* DMA_16_Write)(System::UInt32 chan,System::Byte * data,System::UInt16 size);
#endif


#ifdef MODULE_WANT_MIXER 
/* 	The len here means the amount of samples needed not the buffersize it needed to fill */
typedef void (* MIXER_MixHandler)(System::Byte * sampdate,System::UInt32 len);

/* Different types if modes a mixer channel can work in */
#define MIXER_8MONO		0
#define MIXER_8STEREO	1
#define MIXER_16MONO	2
#define MIXER_16STEREO	3
struct MIXER_Channel;

#define MAX_AUDIO ((1<<(16-1))-1)
#define MIN_AUDIO -(1<<(16-1))

MIXER_Channel *(* MIXER_AddChannel)(MIXER_MixHandler handler,System::UInt32 freq,char * name);
void (* MIXER_SetVolume)(MIXER_Channel * chan,System::Byte vol);
void (* MIXER_SetFreq)(MIXER_Channel * chan,System::UInt32 freq);
void (* MIXER_SetMode)(MIXER_Channel * chan,System::Byte mode);
void (* MIXER_Enable)(MIXER_Channel * chan,bool enable);
#endif

typedef bool (* MODULE_FindHandler)(char * name,void * * function);
typedef char *(* MODULE_StartHandler)(MODULE_FindHandler find_handler);

#define MODULE_START_PROC "ModuleStart"

#ifdef MODULE_START_FUNCTION
#include <stdio.h>

#define GET_FUNCTION(a)													\
	if (!find_handler(#a ,(void * *) &a)) {								\
		return "Can't find requested function";							\
	};


#if defined (WIN32)
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern "C" {
__declspec(dllexport) 
#endif 
char * ModuleStart (MODULE_FindHandler find_handler) {

#ifdef MODULE_WANT_IRQ_EOI
GET_FUNCTION(IRQ_RegisterEOIHandler);
GET_FUNCTION(IRQ_FreeEOIHandler);
#endif 

#ifdef MODULE_WANT_IRQ 
GET_FUNCTION(IRQ_Activate);
GET_FUNCTION(IRQ_Deactivate);
#endif

#ifdef MODULE_WANT_IO_READ
GET_FUNCTION(IO_RegisterReadHandler);
GET_FUNCTION(IO_FreeReadHandler);
#endif

#ifdef MODULE_WANT_IO_WRITE
GET_FUNCTION(IO_RegisterWriteHandler);
GET_FUNCTION(IO_FreeWriteHandler);
#endif

#ifdef MODULE_WANT_TIMER
GET_FUNCTION(TIMER_RegisterMicroHandler);
#endif

#ifdef MODULE_WANT_TIMER_TICKS
GET_FUNCTION(TIMER_RegisterTickHandler);
#endif

#ifdef MODULE_WANT_DMA_READ
GET_FUNCTION(DMA_8_Read);
GET_FUNCTION(DMA_16_Read);
#endif

#ifdef MODULE_WANT_DMA_WRITE
GET_FUNCTION(DMA_8_Write);
GET_FUNCTION(DMA_16_Write);
#endif

#ifdef MODULE_WANT_MIXER
GET_FUNCTION(MIXER_AddChannel);
GET_FUNCTION(MIXER_SetVolume);
GET_FUNCTION(MIXER_SetFreq);
GET_FUNCTION(MIXER_SetMode);
GET_FUNCTION(MIXER_Enable);
#endif

return MODULE_START_FUNCTION;

}
#if defined (WIN32)
}
#endif 



#endif

