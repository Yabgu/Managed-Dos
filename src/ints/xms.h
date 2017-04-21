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

#ifndef __XMS_H__
#define __XMS_H__

unsigned	XMS_QueryFreeMemory		(System::UInt16& largestFree, System::UInt16& totalFree);
unsigned	XMS_AllocateMemory		(unsigned size, System::UInt16& handle);
unsigned	XMS_FreeMemory			(unsigned handle);
unsigned	XMS_MoveMemory			(PhysPt bpt);
unsigned	XMS_LockMemory			(unsigned handle, System::UInt32& address);
unsigned	XMS_UnlockMemory		(unsigned handle);
unsigned	XMS_GetHandleInformation(unsigned handle, System::Byte& lockCount, System::Byte& numFree, System::UInt16& size);
unsigned	XMS_ResizeMemory		(unsigned handle, unsigned newSize);

unsigned	XMS_EnableA20			(bool enable);
unsigned	XMS_GetEnabledA20		(void);

#endif
