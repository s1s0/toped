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

#include "tpdph.h"
#include <string>
#include <sstream>
#include "outbox.h"
#include "ps_out.h"

PSFile::PSFile(std::string fname)
{
   _fname = fname;
   _hierarchical = false;
   std::ostringstream info;
   info << "Writing PostScript output file: "<< _fname;
   tell_log(console::MT_INFO,info.str());
	std::string filename = convertString(_fname);
   if (!(_psfh = fopen(filename.c_str(),"wt")))
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
   fprintf(_psfh,"/tr{gsave concat cvx exec grestore}bd\n");
   fprintf(_psfh,"/cn{gsave concat}bd\n");
   fprintf(_psfh,"/gr{grestore}bd\n");
   fprintf(_psfh,"/dt{gsave selectfont moveto show grestore}bd\n");
   fprintf(_psfh,"/dp{gsave setlinecap setlinewidth ustrokepath false upath grestore dpl}bd\n");
   fprintf(_psfh,"/dc_ {ustroke}bd\n");
   fprintf(_psfh,"/tc_ {0.5 0.5 0.5 setrgbcolor}bd\n");
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

void PSFile::defineFill(std::string pname, const byte* pat)
{
   fprintf(_psfh,"<< /PatternType 1\n");
   fprintf(_psfh,"   /PaintType 2\n");
   fprintf(_psfh,"   /TilingType 1\n");
   fprintf(_psfh,"   /BBox [0 0 32 32]\n");
   fprintf(_psfh,"   /XStep 32\n");
   fprintf(_psfh,"   /YStep 32\n");
   fprintf(_psfh,"   /PaintProc\n");
   fprintf(_psfh,"    { pop\n");
   fprintf(_psfh,"      32 32\n");
   fprintf(_psfh,"      true\n");
   fprintf(_psfh,"      [1 0 0 1 0 0]\n");
   fprintf(_psfh,"      {<");
   for(word i = 0; i < 32; i++)
   {
      if ((0 == i%4) && (i != 31))
         fprintf(_psfh,"\n          ");
      fprintf(_psfh,"%02x%02x%02x%02x", pat[4*i+0], pat[4*i+1], pat[4*i+2], pat[4*i+3] );
   }
   fprintf(_psfh,"\n      >}\n");
   fprintf(_psfh,"      imagemask\n");
   fprintf(_psfh,"      fill\n");
   fprintf(_psfh,"    } bind\n");
   fprintf(_psfh,">>\n");
   fprintf(_psfh,"matrix\n");
   fprintf(_psfh,"makepattern\n");
   fprintf(_psfh,"/tp_%s exch def\n",pname.c_str());
   fprintf(_psfh,"/dc_%s {gsave dup ustroke currentrgbcolor tp_%s setpattern ufill grestore}bd\n",
           pname.c_str(),
           pname.c_str()
          );
}

void PSFile::cellHeader(std::string cellname, DBbox overlap)
{
   if (_hierarchical)
   {
      fprintf(_psfh, "%%Cell %s\n", cellname.c_str());
      fprintf(_psfh, "/%s{\n", cellname.c_str());
   }
}

void PSFile::cellFooter()
{
   if (_hierarchical)
      fprintf(_psfh,"} bd\n");
   else
      fprintf(_psfh,"gr\n");
}

void PSFile::propSet(std::string color_name, std::string pattern_name)
{
   fprintf(_psfh, "      tc_%s\n", color_name.c_str());
   fprintf(_psfh, "      /dpl {dc_%s} bd\n", pattern_name.c_str());
}

void PSFile::poly(const int4b* pdata, unsigned psize, const DBbox bbox)
{
   fprintf(_psfh,"      {{%i %i %i %i ", bbox.p1().x(), bbox.p1().y(),
                                            bbox.p2().x(), bbox.p2().y() );
   for(word i = 0; i < psize; i++)
      fprintf(_psfh,"%i %i ",pdata[2*i], pdata[2*i+1]);
   fprintf(_psfh,"}<00 01 %X 03 0A>}dpl\n",31 + psize);
}

void PSFile::wire(const int4b* const pdata, unsigned psize, word width, DBbox bbox)
{
   fprintf(_psfh,"      {{%i %i %i %i ", bbox.p1().x(), bbox.p1().y(),
                                            bbox.p2().x(), bbox.p2().y() );
   for(word i = 0; i < psize; i++)
      fprintf(_psfh,"%i %i ",pdata[2*i], pdata[2*i+1]);
   //It's possible here to specify the pathtype of GDSII style
   //int pt = (4== pathtype) ? 2 : pathtype;
   // in Toped however we have only one pathtype - which is equivalent to type 2
   // in both - PS and GDSII
   fprintf(_psfh,"}<00 01 %X 03>} %i %i dp\n",31+psize, width, 2/*pt*/);
}

void PSFile::text(std::string text, const CTM tmtrx)
{
   fprintf(_psfh,"(%s) %G %G /Helvetica [%G %G %G %G %G %G] dt\n",text.c_str(),
                     tmtrx.tx(), tmtrx.ty(),
                     tmtrx.a(), tmtrx.b(), tmtrx.c(), tmtrx.d(), 0.0, 0.0);
}

void PSFile::cellref(std::string cellname, const CTM mx)
{
   if (_hierarchical)
      fprintf(_psfh,"      /%s [%G %G %G %G %G %G] tr\n", cellname.c_str(),
                           mx.a(), mx.b(), mx.c(), mx.d(), mx.tx(), mx.ty());
   else
   {
      fprintf(_psfh,"      [%G %G %G %G %G %G] cn\n",
                           mx.a(), mx.b(), mx.c(), mx.d(), mx.tx(), mx.ty());
   }

}

void PSFile::pspage_header(const DBbox box)
{
   double W=((220.0 - 40.0)/25.4)*72.0;
   double H=((297.0 - 40.0)/25.4)*72.0;
   double w = fabs(double(box.p1().x() - box.p2().x()));
   double h = fabs(double(box.p1().y() - box.p2().y()));
   double sc = (W/H < w/h) ? w/W : h/H;
   double tx = ((box.p1().x() + box.p2().x()) - ( W   * sc) ) / 2;
   double ty = ((box.p1().y() + box.p2().y()) - ( H   * sc) ) / 2;
   CTM laymx( sc, 0.0, 0.0, sc, tx, ty);
   CTM psmx(laymx.Reversed());
   psmx.Translate(20.0 * 72.0 / 25.4, 20.0 * 72.0 / 25.4);

   fprintf(_psfh,"%%%%EndProlog\n");
   fprintf(_psfh,"[%G %G %G %G %G %G] concat\n",
                         psmx.a(), psmx.b(), psmx.c(), psmx.d(), psmx.tx(), psmx.ty());
   fprintf(_psfh,"[/Pattern /DeviceRGB] setcolorspace\n");
}

void PSFile::pspage_footer(std::string topcell)
{
   if (_hierarchical)
      fprintf(_psfh,"%s\n",topcell.c_str());
   fprintf(_psfh,"showpage\n");
   fprintf(_psfh,"%%%%EOF\n");
}

PSFile::~PSFile()
{
   fclose(_psfh);
}
