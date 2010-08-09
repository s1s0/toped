//===========================================================================
//                                                                          =
//   This program is free software; you can redistribute it and/or modify   =
//   it under the terms of the GNU General Public License as published by   =
//   the Free Software Foundation; either version 2 of the License, or      =
//   (at your option) any later version.                                    =
// ------------------------------------------------------------------------ =
//                  TTTTT    OOO    PPPP    EEEE    DDDD                    =
//                  T T T   O   O   P   P   E       D   D                   =
//                    T    O     O  PPPP    EEE     D    D                  =
//                    T     O   O   P       E       D   D                   =
//                    T      OOO    P       EEEEE   DDDD                    =
//                                                                          =
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL: https://toped.googlecode.com/svn/branches/MULTI_DB2/tpd_DB/tedat_ext.h $
//        Created: Sun Aug 9 2010
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Layout primitives with extentions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1559 $
//          $Date: 2010-06-29 00:03:32 +0300 (Âò, 29 èþí 2010) $
//        $Author: krustev.svilen $
//===========================================================================

#ifndef TEDAT_EXT_H
#define TEDAT_EXT_H
#include "tedat.h"

namespace laydata 
{
	class TdtBoxEXT : public TdtBox 
	{
	public:
						TdtBoxEXT(const TP& p1, const TP& p2):TdtBox(p1, p2) {};
						TdtBoxEXT(TEDfile* const tedfile):TdtBox(tedfile) {};
						~TdtBoxEXT() {};
		void			setInt(int extInt);
		int			getInt(void);
		void			setLong(long extLong);
		long			getLong(void);
		void			setString(const std::string &extString);
		std::string getString(void);
		void			setClientData(void* clientData);
		void*			getClientData(void);
	private:
		int			_extInt;
		long			_extLong;
		std::string _extString;
		void*			_clientData;
	};

   class TdtPolyEXT : public TdtPoly   
	{
    public:
						TdtPolyEXT(const pointlist& plist):TdtPoly(plist) {};
						TdtPolyEXT(int4b* plist, unsigned psize):TdtPoly(plist, psize) {};
						TdtPolyEXT(TEDfile* const tedfile):TdtPoly(tedfile) {};
						~TdtPolyEXT() {};
		void			setInt(int extInt);
		int			getInt(void);
		void			setLong(long extLong);
		long			getLong(void);
		void			setString(const std::string &extString);
		std::string getString(void);
		void			setClientData(void* clientData);
		void*			getClientData(void);
	private:
		int			_extInt;
		long			_extLong;
		std::string _extString;
		void*			_clientData;
	};
}
#endif //TEDAT_EXT_H