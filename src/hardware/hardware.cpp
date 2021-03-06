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

/* $Id: hardware.cpp,v 1.23 2009-10-11 18:09:22 qbix79 Exp $ */

#include <msclr/auto_handle.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dosbox.h"
#include "hardware.h"
#include "setup.h"
#include "support.h"
#include "mem.h"
#include "mapper.h"
#include "pic.h"
#include "render.h"
#include "cross.h"

#if (C_SSHOT)
#include "../libs/zmbv/zmbv.cpp"
#endif

static std::string capturedir;
extern const char* RunningProgram;
unsigned CaptureState;

#define WAVE_BUF 16*1024
#define MIDI_BUF 4*1024
#define AVI_HEADER_SIZE	500

static struct {
	struct {
		FILE * handle;
		System::Int16 buf[WAVE_BUF][2];
		unsigned used;
		System::UInt32 length;
		System::UInt32 freq;
	} wave; 
	struct {
		FILE * handle;
		System::Byte buffer[MIDI_BUF];
		unsigned used,done;
		System::UInt32 last;
	} midi;
	struct {
		unsigned rowlen;
	} image;
#if (C_SSHOT)
	struct {
		FILE		*handle;
		unsigned		frames;
		System::Int16		audiobuf[WAVE_BUF][2];
		unsigned		audioused;
		unsigned		audiorate;
		unsigned		audiowritten;
		VideoCodec	*codec;
		unsigned		width, height, bpp;
		unsigned		written;
		float		fps;
		int			bufSize;
		void		*buf;
		System::Byte		*index;
		unsigned		indexsize, indexused;
	} video;
#endif
} capture;


System::String^ OpenCaptureFilePath(System::String^ type, System::String^ ext) {
	if (capturedir.empty()) {
		LOG_MSG("Please specify a capture directory");
		return nullptr;
	}

	System::String^ captureManagedDir = gcnew System::String(capturedir.c_str());

	System::String^ RunningProgramManaged = gcnew System::String(RunningProgram);

	unsigned last = 0;
	char file_start[16];
	dir_information * dir;
	/* Find a filename to open */
	dir = open_directory(capturedir.c_str());
	if (!dir) {
		//Try creating it first
		Cross::CreateDir(capturedir);
		dir = open_directory(capturedir.c_str());
		if (!dir) {

			LOG_MSG("Can't open dir %s for capturing %s", capturedir.c_str(), type);
			return nullptr;
		}
	}

	System::String^ searchPattern = System::String::Format("{0}_*{1}", RunningProgramManaged, ext);
	
	int num = System::Linq::Enumerable::Count( System::IO::Directory::EnumerateFiles(captureManagedDir, searchPattern));
	
	
	return System::String::Format("{0}{1}{2}_{3}{4}", captureManagedDir, CROSS_FILESPLIT, RunningProgramManaged, last, ext);
}

FILE * OpenCaptureFile(const char * type,const char * ext) {
	if (capturedir.empty()) {
		LOG_MSG("Please specify a capture directory");
		return 0;
	}

	unsigned last = 0;
	char file_start[16];
	dir_information * dir;
	/* Find a filename to open */
	dir = open_directory(capturedir.c_str());
	if (!dir) {
		//Try creating it first
		Cross::CreateDir(capturedir);
		dir = open_directory(capturedir.c_str());
		if (!dir) {

			LOG_MSG("Can't open dir %s for capturing %s", capturedir.c_str(), type);
			return 0;
		}
	}
	strcpy(file_start, RunningProgram);
	lowcase(file_start);
	strcat(file_start, "_");
	bool is_directory;
	char tempname[CROSS_LEN];
	bool testRead = read_directory_first(dir, tempname, is_directory);
	for (; testRead; testRead = read_directory_next(dir, tempname, is_directory)) {
		char * test = strstr(tempname, ext);
		if (!test || strlen(test) != strlen(ext))
			continue;
		*test = 0;
		if (strncasecmp(tempname, file_start, strlen(file_start)) != 0) continue;
		unsigned num = atoi(&tempname[strlen(file_start)]);
		if (num >= last) last = num + 1;
	}
	close_directory(dir);
	char file_name[CROSS_LEN];
	sprintf(file_name, "%s%c%s%03d%s", capturedir.c_str(), CROSS_FILESPLIT, file_start, last, ext);
	/* Open the actual file */
	FILE * handle=fopen(file_name,"wb");
	if (handle) {
		LOG_MSG("Capturing %s to %s",type,file_name);
	} else {
		LOG_MSG("Failed to open %s for capturing %s",file_name,type);
	}
	return handle;
}

#if (C_SSHOT)
static void CAPTURE_AddAviChunk(const char * tag, System::UInt32 size, void * data, System::UInt32 flags) {
	System::Byte chunk[8];System::Byte *index;System::UInt32 pos, writesize;

	chunk[0] = tag[0];chunk[1] = tag[1];chunk[2] = tag[2];chunk[3] = tag[3];
	host_writed(&chunk[4], size);   
	/* Write the actual data */
	fwrite(chunk,1,8,capture.video.handle);
	writesize = (size+1)&~1;
	fwrite(data,1,writesize,capture.video.handle);
	pos = capture.video.written + 4;
	capture.video.written += writesize + 8;
	if (capture.video.indexused + 16 >= capture.video.indexsize ) {
		capture.video.index = (System::Byte*)realloc( capture.video.index, capture.video.indexsize + 16 * 4096);
		if (!capture.video.index)
			E_Exit("Ran out of memory during AVI capturing");
		capture.video.indexsize += 16*4096;
	}
	index = capture.video.index+capture.video.indexused;
	capture.video.indexused += 16;
	index[0] = tag[0];
	index[1] = tag[1];
	index[2] = tag[2];
	index[3] = tag[3];
	host_writed(index+4, flags);
	host_writed(index+8, pos);
	host_writed(index+12, size);
}
#endif

#if (C_SSHOT)

static void CAPTURE_VideoHeader() {
		System::Byte avi_header[AVI_HEADER_SIZE];
		unsigned main_list;
		unsigned header_pos=0;
		unsigned save_pos=ftell(capture.video.handle);

#define AVIOUT4(_S_) memcpy(&avi_header[header_pos],_S_,4);header_pos+=4;
#define AVIOUTw(_S_) host_writew(&avi_header[header_pos], _S_);header_pos+=2;
#define AVIOUTd(_S_) host_writed(&avi_header[header_pos], _S_);header_pos+=4;
		/* Try and write an avi header */
		AVIOUT4("RIFF");                    // Riff header 
		AVIOUTd(AVI_HEADER_SIZE + capture.video.written - 8 + capture.video.indexused);
		AVIOUT4("AVI ");
		AVIOUT4("LIST");                    // List header
		main_list = header_pos;
		AVIOUTd(0);				            // TODO size of list
		AVIOUT4("hdrl");

		AVIOUT4("avih");
		AVIOUTd(56);                         /* # of bytes to follow */
		AVIOUTd((System::UInt32)(1000000 / capture.video.fps));       /* Microseconds per frame */
		AVIOUTd(0);
		AVIOUTd(0);                         /* PaddingGranularity (whatever that might be) */
		AVIOUTd(0x110);                     /* Flags,0x10 has index, 0x100 interleaved */
		AVIOUTd(capture.video.frames);      /* TotalFrames */
		AVIOUTd(0);                         /* InitialFrames */
		AVIOUTd(2);                         /* Stream count */
		AVIOUTd(0);                         /* SuggestedBufferSize */
		AVIOUTd(capture.video.width);       /* Width */
		AVIOUTd(capture.video.height);      /* Height */
		AVIOUTd(0);                         /* TimeScale:  Unit used to measure time */
		AVIOUTd(0);                         /* DataRate:   Data rate of playback     */
		AVIOUTd(0);                         /* StartTime:  Starting time of AVI data */
		AVIOUTd(0);                         /* DataLength: Size of AVI data chunk    */

		/* Video stream list */
		AVIOUT4("LIST");
		AVIOUTd(4 + 8 + 56 + 8 + 40);       /* Size of the list */
		AVIOUT4("strl");
		/* video stream header */
        AVIOUT4("strh");
		AVIOUTd(56);                        /* # of bytes to follow */
		AVIOUT4("vids");                    /* Type */
		AVIOUT4(CODEC_4CC);		            /* Handler */
		AVIOUTd(0);                         /* Flags */
		AVIOUTd(0);                         /* Reserved, MS says: wPriority, wLanguage */
		AVIOUTd(0);                         /* InitialFrames */
		AVIOUTd(1000000);                   /* Scale */
		AVIOUTd((System::UInt32)(1000000 * capture.video.fps));              /* Rate: Rate/Scale == samples/second */
		AVIOUTd(0);                         /* Start */
		AVIOUTd(capture.video.frames);      /* Length */
		AVIOUTd(0);                  /* SuggestedBufferSize */
		AVIOUTd(~0);                 /* Quality */
		AVIOUTd(0);                  /* SampleSize */
		AVIOUTd(0);                  /* Frame */
		AVIOUTd(0);                  /* Frame */
		/* The video stream format */
		AVIOUT4("strf");
		AVIOUTd(40);                 /* # of bytes to follow */
		AVIOUTd(40);                 /* Size */
		AVIOUTd(capture.video.width);         /* Width */
		AVIOUTd(capture.video.height);        /* Height */
//		OUTSHRT(1); OUTSHRT(24);     /* Planes, Count */
		AVIOUTd(0);
		AVIOUT4(CODEC_4CC);          /* Compression */
		AVIOUTd(capture.video.width * capture.video.height*4);  /* SizeImage (in bytes?) */
		AVIOUTd(0);                  /* XPelsPerMeter */
		AVIOUTd(0);                  /* YPelsPerMeter */
		AVIOUTd(0);                  /* ClrUsed: Number of colors used */
		AVIOUTd(0);                  /* ClrImportant: Number of colors important */

		/* Audio stream list */
		AVIOUT4("LIST");
		AVIOUTd(4 + 8 + 56 + 8 + 16);  /* Length of list in bytes */
		AVIOUT4("strl");
		/* The audio stream header */
		AVIOUT4("strh");
		AVIOUTd(56);            /* # of bytes to follow */
		AVIOUT4("auds");
		AVIOUTd(0);             /* Format (Optionally) */
		AVIOUTd(0);             /* Flags */
		AVIOUTd(0);             /* Reserved, MS says: wPriority, wLanguage */
		AVIOUTd(0);             /* InitialFrames */
		AVIOUTd(4);    /* Scale */
		AVIOUTd(capture.video.audiorate*4);             /* Rate, actual rate is scale/rate */
		AVIOUTd(0);             /* Start */
		if (!capture.video.audiorate)
			capture.video.audiorate = 1;
		AVIOUTd(capture.video.audiowritten/4);   /* Length */
		AVIOUTd(0);             /* SuggestedBufferSize */
		AVIOUTd(~0);            /* Quality */
		AVIOUTd(4);				/* SampleSize */
		AVIOUTd(0);             /* Frame */
		AVIOUTd(0);             /* Frame */
		/* The audio stream format */
		AVIOUT4("strf");
		AVIOUTd(16);            /* # of bytes to follow */
		AVIOUTw(1);             /* Format, WAVE_ZMBV_FORMAT_PCM */
		AVIOUTw(2);             /* Number of channels */
		AVIOUTd(capture.video.audiorate);          /* SamplesPerSec */
		AVIOUTd(capture.video.audiorate*4);        /* AvgBytesPerSec*/
		AVIOUTw(4);             /* BlockAlign */
		AVIOUTw(16);            /* intPerSample */
		int nmain = header_pos - main_list - 4;
		/* Finish stream list, i.e. put number of bytes in the list to proper pos */

		int njunk = AVI_HEADER_SIZE - 8 - 12 - header_pos;
		AVIOUT4("JUNK");
		AVIOUTd(njunk);
		/* Fix the size of the main list */
		header_pos = main_list;
		AVIOUTd(nmain);
		header_pos = AVI_HEADER_SIZE - 12;
		AVIOUT4("LIST");
		AVIOUTd(capture.video.written+4); /* Length of list in bytes */
		AVIOUT4("movi");
		/* First add the index table to the end */
		memcpy(capture.video.index, "idx1", 4);
		host_writed( capture.video.index+4, capture.video.indexused - 8 );
		fwrite( capture.video.index, 1, capture.video.indexused, capture.video.handle);
		fseek(capture.video.handle, 0, SEEK_SET);
		fwrite(&avi_header, 1, AVI_HEADER_SIZE, capture.video.handle);
		fseek(capture.video.handle, save_pos, SEEK_SET);
}

static void CAPTURE_VideoEvent(bool pressed) {
	if (!pressed)
		return;
	if (CaptureState & CAPTURE_VIDEO) {
		/* Close the video */
		CaptureState &= ~CAPTURE_VIDEO;
		LOG_MSG("Stopped capturing video.");	

		/* Adds AVI header to the file */
		CAPTURE_VideoHeader();

		fclose( capture.video.handle );
		free( capture.video.index );
		free( capture.video.buf );
		delete capture.video.codec;
		capture.video.handle = 0;
	} else {
		CaptureState |= CAPTURE_VIDEO;
	}
}
#endif

void CAPTURE_AddImage(unsigned width, unsigned height, unsigned bpp, unsigned pitch, unsigned flags, float fps, System::Byte * data, System::Byte * pal) {
#if (C_SSHOT)
	if (height > SCALER_MAXHEIGHT)
		return;
	if (width > SCALER_MAXWIDTH)
		return;

	int stride = ((width + 1) / 2) * 2;
	System::IntPtr dataPtr(data);

	if (CaptureState & CAPTURE_IMAGE) {
		System::Drawing::Bitmap^ bitmap;

		switch (bpp) {
		case 8:
			bitmap = gcnew System::Drawing::Bitmap(width, height, stride, System::Drawing::Imaging::PixelFormat::Format8bppIndexed, dataPtr);
			if (pal != nullptr) {
				auto palette = bitmap->Palette->Entries;
				for (int i = 0; i < 256; i++) {
					palette[i] = System::Drawing::Color::FromArgb(255, pal[i * 4 + 0], pal[i * 4 + 1], pal[i * 4 + 2]);
				}
			}
			break;
		case 15:
			bitmap = gcnew System::Drawing::Bitmap(width, height, ((width + 1) / 2) * 2, System::Drawing::Imaging::PixelFormat::Format16bppArgb1555, dataPtr);
			break;
		case 16:
			bitmap = gcnew System::Drawing::Bitmap(width, height, ((width + 1) / 2) * 2, System::Drawing::Imaging::PixelFormat::Format16bppRgb565, dataPtr);
			break;
		case 32:
			bitmap = gcnew System::Drawing::Bitmap(width, height, ((width + 1) / 2) * 2, System::Drawing::Imaging::PixelFormat::Format32bppArgb, dataPtr);
			break;
		}

		CaptureState &= ~CAPTURE_IMAGE;
		try
		{
			auto fileName = OpenCaptureFilePath("Screenshot", ".png");

			/* Open the actual file */
			msclr::auto_handle<System::IO::FileStream> fs(gcnew System::IO::FileStream(fileName, System::IO::FileMode::CreateNew));
			bitmap->Save(fs, System::Drawing::Imaging::ImageFormat::Png);
		}
		catch (System::Exception^ ex)
		{
		}
	}
skip_shot:
#if 0
	if (CaptureState & CAPTURE_VIDEO) {
		zmbv_format_t format;
		/* Disable capturing if any of the test fails */
		if (capture.video.handle && (
			capture.video.width != width ||
			capture.video.height != height ||
			capture.video.bpp != bpp ||
			capture.video.fps != fps)) 
		{
			CAPTURE_VideoEvent(true);
		}
		CaptureState &= ~CAPTURE_VIDEO;
		switch (bpp) {
		case 8:format = ZMBV_FORMAT_8BPP;break;
		case 15:format = ZMBV_FORMAT_15BPP;break;
		case 16:format = ZMBV_FORMAT_16BPP;break;
		case 32:format = ZMBV_FORMAT_32BPP;break;
		default:
			goto skip_video;
		}
		if (!capture.video.handle) {
			capture.video.handle = OpenCaptureFile("Video",".avi");
			if (!capture.video.handle)
				goto skip_video;
			capture.video.codec = new VideoCodec();
			if (!capture.video.codec)
				goto skip_video;
			if (!capture.video.codec->SetupCompress( width, height)) 
				goto skip_video;
			capture.video.bufSize = capture.video.codec->NeededSize(width, height, format);
			capture.video.buf = malloc( capture.video.bufSize );
			if (!capture.video.buf)
				goto skip_video;
			capture.video.index = (System::Byte*)malloc( 16*4096 );
			if (!capture.video.buf)
				goto skip_video;
			capture.video.indexsize = 16*4096;
			capture.video.indexused = 8;

			capture.video.width = width;
			capture.video.height = height;
			capture.video.bpp = bpp;
			capture.video.fps = fps;
			for (int i=0;i<AVI_HEADER_SIZE;i++)
				fputc(0,capture.video.handle);
			capture.video.frames = 0;
			capture.video.written = 0;
			capture.video.audioused = 0;
			capture.video.audiowritten = 0;
		}
		int codecFlags;
		if (capture.video.frames % 300 == 0)
			codecFlags = 1;
		else codecFlags = 0;
		if (!capture.video.codec->PrepareCompressFrame( codecFlags, format, (char *)pal, capture.video.buf, capture.video.bufSize))
			goto skip_video;

		for (int i=0;i<height;i++) {
			void * rowPointer;
			if (flags & CAPTURE_FLAG_DBLW) {
				void *srcLine;
				unsigned x;
				unsigned countWidth = width >> 1;
				if (flags & CAPTURE_FLAG_DBLH)
					srcLine=(data+(i >> 1)*pitch);
				else
					srcLine=(data+(i >> 0)*pitch);
				switch ( bpp) {
				case 8:
					for (x=0;x<countWidth;x++)
						((System::Byte *)doubleRow)[x*2+0] =
						((System::Byte *)doubleRow)[x*2+1] = ((System::Byte *)srcLine)[x];
					break;
				case 15:
				case 16:
					for (x=0;x<countWidth;x++)
						((System::UInt16 *)doubleRow)[x*2+0] =
						((System::UInt16 *)doubleRow)[x*2+1] = ((System::UInt16 *)srcLine)[x];
					break;
				case 32:
					for (x=0;x<countWidth;x++)
						((System::UInt32 *)doubleRow)[x*2+0] =
						((System::UInt32 *)doubleRow)[x*2+1] = ((System::UInt32 *)srcLine)[x];
					break;
				}
                rowPointer=doubleRow;
			} else {
				if (flags & CAPTURE_FLAG_DBLH)
					rowPointer=(data+(i >> 1)*pitch);
				else
					rowPointer=(data+(i >> 0)*pitch);
			}
			capture.video.codec->CompressLines( 1, &rowPointer );
		}
		int written = capture.video.codec->FinishCompressFrame();
		if (written < 0)
			goto skip_video;
		CAPTURE_AddAviChunk( "00dc", written, capture.video.buf, codecFlags & 1 ? 0x10 : 0x0);
		capture.video.frames++;
//		LOG_MSG("Frame %d video %d audio %d",capture.video.frames, written, capture.video.audioused *4 );
		if ( capture.video.audioused ) {
			CAPTURE_AddAviChunk( "01wb", capture.video.audioused * 4, capture.video.audiobuf, 0);
			capture.video.audiowritten = capture.video.audioused*4;
			capture.video.audioused = 0;
		}

		/* Adds AVI header to the file */
		CAPTURE_VideoHeader();

		/* Everything went okay, set flag again for next frame */
		CaptureState |= CAPTURE_VIDEO;
	}
#endif
skip_video:
#endif
	return;
}


#if (C_SSHOT)
static void CAPTURE_ScreenShotEvent(bool pressed) {
	if (!pressed)
		return;
	CaptureState |= CAPTURE_IMAGE;
}
#endif


/* WAV capturing */
static System::Byte wavheader[]={
	'R','I','F','F',	0x0,0x0,0x0,0x0,		/* System::UInt32 Riff Chunk ID /  System::UInt32 riff size */
	'W','A','V','E',	'f','m','t',' ',		/* System::UInt32 Riff Format  / System::UInt32 fmt chunk id */
	0x10,0x0,0x0,0x0,	0x1,0x0,0x2,0x0,		/* System::UInt32 fmt size / System::UInt16 encoding/ System::UInt16 channels */
	0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,		/* System::UInt32 freq / System::UInt32 byterate */
	0x4,0x0,0x10,0x0,	'd','a','t','a',		/* System::UInt16 byte-block / System::UInt16 int / System::UInt16 data chunk id */
	0x0,0x0,0x0,0x0,							/* System::UInt32 data size */
};

void CAPTURE_AddWave(System::UInt32 freq, System::UInt32 len, System::Int16 * data) {
#if (C_SSHOT)
	if (CaptureState & CAPTURE_VIDEO) {
		unsigned left = WAVE_BUF - capture.video.audioused;
		if (left > len)
			left = len;
		memcpy( &capture.video.audiobuf[capture.video.audioused], data, left*4);
		capture.video.audioused += left;
		capture.video.audiorate = freq;
	}
#endif
	if (CaptureState & CAPTURE_WAVE) {
		if (!capture.wave.handle) {
			capture.wave.handle=OpenCaptureFile("Wave Output",".wav");
			if (!capture.wave.handle) {
				CaptureState &= ~CAPTURE_WAVE;
				return;
			}
			capture.wave.length = 0;
			capture.wave.used = 0;
			capture.wave.freq = freq;
			fwrite(wavheader,1,sizeof(wavheader),capture.wave.handle);
		}
		System::Int16 * read = data;
		while (len > 0 ) {
			unsigned left = WAVE_BUF - capture.wave.used;
			if (!left) {
				fwrite(capture.wave.buf,1,4*WAVE_BUF,capture.wave.handle);
				capture.wave.length += 4*WAVE_BUF;
				capture.wave.used = 0;
				left = WAVE_BUF;
			}
			if (left > len)
				left = len;
			memcpy( &capture.wave.buf[capture.wave.used], read, left*4);
			capture.wave.used += left;
			read += left*2;
			len -= left;
		}
	}
}
static void CAPTURE_WaveEvent(bool pressed) {
	if (!pressed)
		return;
	/* Check for previously opened wave file */
	if (capture.wave.handle) {
		LOG_MSG("Stopped capturing wave output.");
		/* Write last piece of audio in buffer */
		fwrite(capture.wave.buf,1,capture.wave.used*4,capture.wave.handle);
		capture.wave.length+=capture.wave.used*4;
		/* Fill in the header with useful information */
		host_writed(&wavheader[0x04],capture.wave.length+sizeof(wavheader)-8);
		host_writed(&wavheader[0x18],capture.wave.freq);
		host_writed(&wavheader[0x1C],capture.wave.freq*4);
		host_writed(&wavheader[0x28],capture.wave.length);
		
		fseek(capture.wave.handle,0,0);
		fwrite(wavheader,1,sizeof(wavheader),capture.wave.handle);
		fclose(capture.wave.handle);
		capture.wave.handle=0;
		CaptureState |= CAPTURE_WAVE;
	} 
	CaptureState ^= CAPTURE_WAVE;
}

/* MIDI capturing */

static System::Byte midi_header[]={
	'M','T','h','d',			/* System::UInt32, Header Chunk */
	0x0,0x0,0x0,0x6,			/* System::UInt32, Chunk Length */
	0x0,0x0,					/* System::UInt16, Format, 0=single track */
	0x0,0x1,					/* System::UInt16, Track Count, 1 track */
	0x01,0xf4,					/* System::UInt16, Timing, 2 beats/second with 500 frames */
	'M','T','r','k',			/* System::UInt32, Track Chunk */
	0x0,0x0,0x0,0x0,			/* System::UInt32, Chunk Length */
	//Track data
};


static void RawMidiAdd(System::Byte data) {
	capture.midi.buffer[capture.midi.used++]=data;
	if (capture.midi.used >= MIDI_BUF ) {
		capture.midi.done += capture.midi.used;
		fwrite(capture.midi.buffer,1,MIDI_BUF,capture.midi.handle);
		capture.midi.used = 0;
	}
}

static void RawMidiAddNumber(System::UInt32 val) {
	if (val & 0xfe00000) RawMidiAdd((System::Byte)(0x80|((val >> 21) & 0x7f)));
	if (val & 0xfffc000) RawMidiAdd((System::Byte)(0x80|((val >> 14) & 0x7f)));
	if (val & 0xfffff80) RawMidiAdd((System::Byte)(0x80|((val >> 7) & 0x7f)));
	RawMidiAdd((System::Byte)(val & 0x7f));
}

void CAPTURE_AddMidi(bool sysex, unsigned len, System::Byte * data) {
	if (!capture.midi.handle) {
		capture.midi.handle=OpenCaptureFile("Raw Midi",".mid");
		if (!capture.midi.handle) {
			return;
		}
		fwrite(midi_header,1,sizeof(midi_header),capture.midi.handle);
		capture.midi.last=PIC_Ticks;
	}
	System::UInt32 delta=PIC_Ticks-capture.midi.last;
	capture.midi.last=PIC_Ticks;
	RawMidiAddNumber(delta);
	if (sysex) {
		RawMidiAdd( 0xf0 );
		RawMidiAddNumber( len );
	}
	for (unsigned i=0;i<len;i++) 
		RawMidiAdd(data[i]);
}

static void CAPTURE_MidiEvent(bool pressed) {
	if (!pressed)
		return;
	/* Check for previously opened wave file */
	if (capture.midi.handle) {
		LOG_MSG("Stopping raw midi saving and finalizing file.");
		//Delta time
		RawMidiAdd(0x00);
		//End of track event
		RawMidiAdd(0xff);
		RawMidiAdd(0x2F);
		RawMidiAdd(0x00);
		/* clear out the final data in the buffer if any */
		fwrite(capture.midi.buffer,1,capture.midi.used,capture.midi.handle);
		capture.midi.done+=capture.midi.used;
		fseek(capture.midi.handle,18, SEEK_SET);
		System::Byte size[4];
		size[0]=(System::Byte)(capture.midi.done >> 24);
		size[1]=(System::Byte)(capture.midi.done >> 16);
		size[2]=(System::Byte)(capture.midi.done >> 8);
		size[3]=(System::Byte)(capture.midi.done >> 0);
		fwrite(&size,1,4,capture.midi.handle);
		fclose(capture.midi.handle);
		capture.midi.handle=0;
		CaptureState &= ~CAPTURE_MIDI;
		return;
	} 
	CaptureState ^= CAPTURE_MIDI;
	if (CaptureState & CAPTURE_MIDI) {
		LOG_MSG("Preparing for raw midi capture, will start with first data.");
		capture.midi.used=0;
		capture.midi.done=0;
		capture.midi.handle=0;
	} else {
		LOG_MSG("Stopped capturing raw midi before any data arrived.");
	}
}

class HARDWARE:public Module_base{
public:
	HARDWARE(Section* configuration):Module_base(configuration){
		Section_prop * section = static_cast<Section_prop *>(configuration);
		Prop_path* proppath= section->Get_path("captures");
		capturedir = proppath->realpath;
		CaptureState = 0;
		MAPPER_AddHandler(CAPTURE_WaveEvent,MK_f6,MMOD1,"recwave","Rec Wave");
		MAPPER_AddHandler(CAPTURE_MidiEvent,MK_f8,MMOD1|MMOD2,"caprawmidi","Cap MIDI");
#if (C_SSHOT)
		MAPPER_AddHandler(CAPTURE_ScreenShotEvent,MK_f5,MMOD1,"scrshot","Screenshot");
		MAPPER_AddHandler(CAPTURE_VideoEvent,MK_f5,MMOD1|MMOD2,"video","Video");
#endif
	}
	~HARDWARE(){
		if (capture.wave.handle) CAPTURE_WaveEvent(true);
		if (capture.midi.handle) CAPTURE_MidiEvent(true);
	}
};

static HARDWARE* test;

void HARDWARE_Destroy(Section * sec) {
	delete test;
}

void HARDWARE_Init(Section * sec) {
	test = new HARDWARE(sec);
	sec->AddDestroyFunction(&HARDWARE_Destroy,true);
}
