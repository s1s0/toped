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
//   This file is a part of Toped project (C) 2001-2006 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Jan 07 2007
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Post Script output
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <string>
#include <sstream>
#include "../tpd_common/outbox.h"
#include "ps_out.h"

PSFile::PSFile(std::string fname)
{
   _fname = fname;
   std::ostringstream info;
   info << "Writing PostScript output file: "<< _fname;
   tell_log(console::MT_INFO,info.str());
   if (!(_psfh = fopen(_fname.c_str(),"w")))
   {// file can't be opened for writing errcode = errno;
      std::ostringstream info;
      info << "File "<< _fname <<" can NOT be opened";
      tell_log(console::MT_ERROR,info.str());
      //@TODO Exception here
      return;
   }
   TpdTime time_stamp(time(NULL));
   // write file header
   fprintf(_psfh,"%%!PS-Adobe-2.0 \n");
   fprintf(_psfh,"%%%%Title: %s\n",_fname.c_str());
   fprintf(_psfh,"%%%%Creator: Toped rev. ?.?\n");
   fprintf(_psfh,"%%%%Purpose: layout art print\n");
   fprintf(_psfh,"%%%%Date: %s\n",time_stamp().c_str());
   fprintf(_psfh,"%%%%Pages: (atend)\n");
   fprintf(_psfh,"%%%%BoundingBox: (atend)\n");
   fprintf(_psfh,"%%%%EndComments\n");
   writeStdDefs();
}

void PSFile::writeStdDefs()
{
   fprintf(_psfh,"%%%%BeginProlog\n");
   fprintf(_psfh,"/bd{bind def}def\n");
   fprintf(_psfh,"/tr{gsave concat exec grestore}bd\n");
   fprintf(_psfh,"/cn{gsave concat}bd\n");
   fprintf(_psfh,"/gr{grestore}bd\n");
   fprintf(_psfh,"/dt{gsave selectfont moveto show grestore}bd\n");
   fprintf(_psfh,"/dp{gsave setlinecap setlinewidth ustrokepath false upath grestore dpl}bd\n");
}

bool PSFile::checkCellWritten(std::string cellname)
{
   for (nameList::const_iterator i = _childnames.begin();
                                 i != _childnames.end(); i++)
      if (cellname == *i) return true;
   return false;
//   return (_childnames.end() != _childnames.find(cellname));
}

void PSFile::registerCellWritten(std::string cellname)
{
   _childnames.push_back(cellname);
}
   
void PSFile::defineColor(std::string name, byte colR, byte colG, byte colB)
{
   fprintf(_psfh,"/tc_%s {%f %f %f setrgbcolor}bd\n", name.c_str(),
                                                      (real)(colR)/255.0,
                                                      (real)(colG)/255.0,
                                                      (real)(colB)/255.0 );
}

void PSFile::defineFill(std::string, byte*)
{
}

void PSFile::formHeader(std::string cellname, DBbox overlap)
{
   fprintf(_psfh, "%%Cell %s\n", cellname.c_str());
   fprintf(_psfh, "/%s\n", cellname.c_str());
   fprintf(_psfh, "<< /FormType 1\n");
   fprintf(_psfh, "   /BBox [%i %i %i %i]\n", overlap.p1().x(),
                                            overlap.p1().y(),
                                            overlap.p2().x(),
                                            overlap.p2().y() );
   fprintf(_psfh, "   /Matrix [1 0 0 1 0 0]\n");
   fprintf(_psfh, "   /PaintProc {\n");
}

void PSFile::formFooter()
{
   fprintf(_psfh, "   } bind\n");
   fprintf(_psfh, ">> def\n");
}

void PSFile::propSet(std::string color_name, std::string pattern_name)
{
   fprintf(_psfh, "      tc_%s\n", color_name.c_str());
   fprintf(_psfh, "      /dpl {%s} bind\n", pattern_name.c_str());
}

void PSFile::poly(const pointlist points, const DBbox bbox)
{
   fprintf(_psfh,"         {{%i %i %i %i ", bbox.p1().x(), bbox.p1().y(),
                                            bbox.p2().x(), bbox.p2().y() );
   for(word i = 0; i < points.size(); i++)
   {
//      PSP cp = mtrx * wp;//translate wp
      fprintf(_psfh,"%i %i ",points[i].x(), points[i].y());
   }
   fprintf(_psfh,"}<00 01 %X 03 0A>}dpl\n",31+points.size());
}

void PSFile::wire(const pointlist points, word width, DBbox bbox)
{
   fprintf(_psfh,"         {{%i %i %i %i ", bbox.p1().x(), bbox.p1().y(),
                                            bbox.p2().x(), bbox.p2().y() );
   for(word i = 0; i < points.size(); i++)
   {
//      PSP cp = mtrx * wp;//translate fp
      fprintf(_psfh,"%i %i ",points[i].x(), points[i].y());
   }
   //It's possible here to specify the pathtype of GDSII style
   //int pt = (4== pathtype) ? 2 : pathtype;
   fprintf(_psfh,"}<00 01 %X 03>} %i %i dp\n",31+points.size(), width, 1/*pt*/);
}

void PSFile::text(std::string text, const CTM tmtrx)
{
   //PSP cp = tmtrx * wp;// text position
   fprintf(_psfh,"(%s) %G %G /Helvetica [%G %G %G %G %G %G] dt\n",text.c_str(),
                     tmtrx.tx(), tmtrx.ty(),
                     tmtrx.a(), tmtrx.b(), tmtrx.c(), tmtrx.d(), 0.0, 0.0);
}

void PSFile::cellref(std::string text, const CTM)
{
   //@TODO cell reference !!!
}

PSFile::~PSFile()
{
   fclose(_psfh);
}
