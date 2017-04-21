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

extern System::Byte  * lookupRMregb[];
extern System::UInt16 * lookupRMregw[];
extern System::UInt32 * lookupRMregd[];
extern System::Byte  * lookupRMEAregb[256];
extern System::UInt16 * lookupRMEAregw[256];
extern System::UInt32 * lookupRMEAregd[256];

#define GetRM												\
	System::Byte rm=Fetchb();

#define Getrb												\
	System::Byte * rmrb;											\
	rmrb=lookupRMregb[rm];			
	
#define Getrw												\
	System::UInt16 * rmrw;											\
	rmrw=lookupRMregw[rm];			

#define Getrd												\
	System::UInt32 * rmrd;											\
	rmrd=lookupRMregd[rm];			


#define GetRMrb												\
	GetRM;													\
	Getrb;													

#define GetRMrw												\
	GetRM;													\
	Getrw;													

#define GetRMrd												\
	GetRM;													\
	Getrd;													


#define GetEArb												\
	System::Byte * earb=lookupRMEAregb[rm];

#define GetEArw												\
	System::UInt16 * earw=lookupRMEAregw[rm];

#define GetEArd												\
	System::UInt32 * eard=lookupRMEAregd[rm];


