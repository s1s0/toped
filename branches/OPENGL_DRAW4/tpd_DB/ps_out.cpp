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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
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
#include "tedcell.h"

PSFile::PSFile(std::string fname, const layprop::DrawProperties& drawProp)
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
   writeProperties(drawProp);
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

void PSFile::writeProperties(const layprop::DrawProperties& drawProp)
{
   NameList allColors;
   drawProp.allColors(allColors);
   for (NameList::const_iterator CC = allColors.begin(); CC != allColors.end(); CC++)
   {
      layprop::tellRGB coldef(drawProp.getColor(*CC));
      defineColor( CC->c_str() , coldef.red(), coldef.green(), coldef.blue() );
   }
   NameList allPatterns;
   drawProp.allFills(allPatterns);
   for (NameList::const_iterator CC = allPatterns.begin(); CC != allPatterns.end(); CC++)
   {
      defineFill( CC->c_str() , drawProp.getFill(*CC));
   }
}

bool PSFile::checkCellWritten(std::string cellname)
{
   for (NameList::const_iterator i = _childnames.begin();
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

void PSFile::wire(const int4b* const pdata, unsigned psize, WireWidth width, DBbox bbox)
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
//=============================================================================
PsExportFile::PsExportFile(std::string fn, laydata::TdtCell* topcell, /*ExpLayMap* laymap, */const layprop::DrawProperties& drawprop, bool recur) :
   DbExportFile   (fn, topcell, recur),
   _hierarchical  (                  true ), // TODO, remove this option and make all hierarchical
   _childnames    (                       ),
   _totaloverlap  ( topcell->cellOverlap()),
   _drawProp      ( drawprop              )
//   _laymap(laymap)*/
{
   std::string fname(convertString(_fileName));
   _file.open(_fileName.c_str(), std::ios::out);
   //@TODO how to check for an error ?
   // start writing

}

PsExportFile::~PsExportFile()
{
   _file.close();
}

void PsExportFile::libraryStart(std::string libname, TpdTime& libtime, real DBU, real UU)
{
   _file << "%%!PS-Adobe-2.0 \n"
         << "%%%%Title: " << libname << "\n"
         << "%%%%Creator: Toped rev. ?.?\n"
         << "%%%%Purpose: layout art print\n"
         << "%%%%Date: " << libtime() << "\n"
         << "%%%%Pages: (atend)\n"
         << "%%%%BoundingBox: (atend)\n"
         << "%%%%EndComments\n";
   writeStdDefs();
   writeProperties();

   double W=((220.0 - 40.0)/25.4)*72.0;
   double H=((297.0 - 40.0)/25.4)*72.0;
   double w = fabs(double(_totaloverlap.p1().x() - _totaloverlap.p2().x()));
   double h = fabs(double(_totaloverlap.p1().y() - _totaloverlap.p2().y()));
   double sc = (W/H < w/h) ? w/W : h/H;
   double tx = ((_totaloverlap.p1().x() + _totaloverlap.p2().x()) - ( W   * sc) ) / 2;
   double ty = ((_totaloverlap.p1().y() + _totaloverlap.p2().y()) - ( H   * sc) ) / 2;
   CTM laymx( sc, 0.0, 0.0, sc, tx, ty);
   CTM psmx(laymx.Reversed());
   psmx.Translate(20.0 * 72.0 / 25.4, 20.0 * 72.0 / 25.4);

   _file << "%%%%EndProlog\n"
         <<"[" << psmx.a()
         <<" " << psmx.b()
         <<" " << psmx.c()
         <<" " << psmx.d()
         <<" " << psmx.tx()
         <<" " << psmx.ty()
         <<"] concat\n";
   _file << "[/Pattern /DeviceRGB] setcolorspace\n";
}

void PsExportFile::libraryFinish()
{
   if (_hierarchical)
      _file << topcell()->name() << "\n";
   _file << "showpage\n";
   _file << "%%%%EOF\n";

}

void PsExportFile::definitionStart(std::string cellname)
{
   if (_hierarchical)
   {
      _file << "%%Cell " << cellname << "\n"
            << "/"       << cellname << "{\n";
//      fprintf(_psfh, "%%Cell %s\n", cellname.c_str());
//      fprintf(_psfh, "/%s{\n", cellname.c_str());
   }

}

void PsExportFile::definitionFinish()
{
   if (_hierarchical)
      _file << "} bd\n";
//      fprintf(_psfh,"} bd\n");
   else
      _file << "gr\n";
//      fprintf(_psfh,"gr\n");
}

bool PsExportFile::layerSpecification(const LayerDef& laydef)
{
   std::string colorName   = _drawProp.getColorName(laydef);
   std::string patternName = _drawProp.getFillName(laydef);
   _file << "      "
         << "tc_" << colorName
         << "\n";
   _file << "      "
         << "/dpl {dc_" << patternName
         << "} bd\n";
//   fprintf(_psfh, "      tc_%s\n", color_name.c_str());
//   fprintf(_psfh, "      /dpl {dc_%s} bd\n", pattern_name.c_str());
   return true;
}

void PsExportFile::box(const int4b* const pdata)
{
   polygon(pdata, 4);
}

void PsExportFile::polygon(const int4b* const pdata, unsigned psize)
{
   DBbox bbox(pdata[0], pdata[1]) ;
   for (word i = 1; i < psize; i++)
      bbox.overlap(pdata[2*i], pdata[2*i+1]);
   _file << "      "
         << "{{" << bbox.p1().x()
         << " "  << bbox.p1().y()
         << " "  << bbox.p2().x()
         << " "  << bbox.p2().y();
//   fprintf(_psfh,"      {{%i %i %i %i ", ovl.p1().x(), ovl.p1().y(),
//                                         ovl.p2().x(), ovl.p2().y() );
   for(word i = 0; i < psize; i++)
      _file << " " <<  pdata[2*i]
            << " " <<  pdata[2*i+1];
//      fprintf(_psfh,"%i %i ",pdata[2*i], pdata[2*i+1]);
   _file << "}<00 01 " << 31+psize
         << " 03 0A>}"
         << " dpl\n";
//      fprintf(_psfh,"}<00 01 %X 03 0A>}dpl\n",31 + psize);
}

void PsExportFile::wire(const int4b* const pdata, unsigned psize, WireWidth width)
{
   laydata::WireContour wcontour(pdata, psize, width);
   DBbox bbox(wcontour.getCOverlap());
   _file << "      "
         << "{{" << bbox.p1().x()
         << " "  << bbox.p1().y()
         << " "  << bbox.p2().x()
         << " "  << bbox.p2().y();

//   fprintf(_psfh,"      {{%i %i %i %i ", bbox.p1().x(), bbox.p1().y(),
//                                            bbox.p2().x(), bbox.p2().y() );
   for(word i = 0; i < psize; i++)
      _file << " " <<  pdata[2*i]
            << " " <<  pdata[2*i+1];
//      fprintf(_psfh,"%i %i ",pdata[2*i], pdata[2*i+1]);
   //It's possible here to specify the pathtype of GDSII style
   //int pt = (4== pathtype) ? 2 : pathtype;
   // in Toped however we have only one pathtype - which is equivalent to type 2
   // in both - PS and GDSII
   _file << "}<00 01 " << 31+psize
         << " 03>} "   << width
         << " 2 dp\n";
//   fprintf(_psfh,"}<00 01 %X 03>} %i %i dp\n",31+psize, width, 2/*pt*/);
}


void PsExportFile::text(const std::string& text, const CTM& tmtrx)
{
   _file << "("  << text.c_str()
         << ") " << tmtrx.tx()
         << " "  << tmtrx.ty()
         << "/Helvetica "
         << "["  << tmtrx.a()
         << " "  << tmtrx.b()
         << " "  << tmtrx.c()
         << " "  << tmtrx.d()
         << " 0.0 0.0]"
         << "] dt\n";
//   fprintf(_psfh,"(%s) %G %G /Helvetica [%G %G %G %G %G %G] dt\n",text.c_str(),
//                     tmtrx.tx(), tmtrx.ty(),
//                     tmtrx.a(), tmtrx.b(), tmtrx.c(), tmtrx.d(), 0.0, 0.0);
}

void PsExportFile::ref(const std::string& cellname, const CTM& tmtrx)
{
   if (_hierarchical)
      _file << "      "
            << "/"  << cellname.c_str()
            << " [" << tmtrx.a()
            << " "  << tmtrx.b()
            << " "  << tmtrx.c()
            << " "  << tmtrx.d()
            << " "  << tmtrx.tx()
            << " "  << tmtrx.ty()
            << "] tr\n";
//      fprintf(_psfh,"      /%s [%G %G %G %G %G %G] tr\n", cellname.c_str(),
//                           mx.a(), mx.b(), mx.c(), mx.d(), mx.tx(), mx.ty());
   else
      _file << "      "
            << " [" << tmtrx.a()
            << " "  << tmtrx.b()
            << " "  << tmtrx.c()
            << " "  << tmtrx.d()
            << " "  << tmtrx.tx()
            << " "  << tmtrx.ty()
            << "] cn\n";
//      fprintf(_psfh,"      [%G %G %G %G %G %G] cn\n",
//                           mx.a(), mx.b(), mx.c(), mx.d(), mx.tx(), mx.ty());
}

void PsExportFile::aref(const std::string& cellname, const CTM& tmtrx, const laydata::ArrayProps& arrprops)
{
   for (int i = 0; i < arrprops.cols(); i++)
   {// start/stop rows
      for(int j = 0; j < arrprops.rows(); j++)
      { // start/stop columns
         // for each of the visual array figures...
         // ... get the translation matrix ...
         CTM refCTM(arrprops.displ(i,j), 1, 0, false);
         refCTM *= tmtrx;
         ref(cellname, refCTM);
         if (!_hierarchical)
         {
//            _structure->psWrite(psf, drawprop); TODO!
         }
      }
   }

}

bool PsExportFile::checkCellWritten(std::string cellname) const
{
   for (NameList::const_iterator i = _childnames.begin();
                                 i != _childnames.end(); i++)
      if (cellname == *i) return true;
   return false;
//   return (_childnames.end() != _childnames.find(cellname));
}

void PsExportFile::registerCellWritten(std::string cellname)
{
   _childnames.push_back(cellname);
}

void PsExportFile::writeStdDefs()
{
   _file << "%%%%BeginProlog\n"
         << "/bd{bind def}def\n"
         << "/tr{gsave concat cvx exec grestore}bd\n"
         << "/cn{gsave concat}bd\n"
         << "/gr{grestore}bd\n"
         << "/dt{gsave selectfont moveto show grestore}bd\n"
         << "/dp{gsave setlinecap setlinewidth ustrokepath false upath grestore dpl}bd\n"
         << "/dc_ {ustroke}bd\n"
         << "/tc_ {0.5 0.5 0.5 setrgbcolor}bd\n";
}

void PsExportFile::writeProperties()
{
   NameList allColors;
   _drawProp.allColors(allColors);
   for (NameList::const_iterator CC = allColors.begin(); CC != allColors.end(); CC++)
   {
      layprop::tellRGB coldef(_drawProp.getColor(*CC));
      defineColor( CC->c_str() , coldef.red(), coldef.green(), coldef.blue() );
   }
   NameList allPatterns;
   _drawProp.allFills(allPatterns);
   for (NameList::const_iterator CC = allPatterns.begin(); CC != allPatterns.end(); CC++)
   {
      defineFill( CC->c_str() , _drawProp.getFill(*CC));
   }
}

void PsExportFile::defineColor(std::string name, byte colR, byte colG, byte colB)
{
   _file << "/tc_" << name
         << "{"    << (real)(colR)/255.0
         << " "    << (real)(colG)/255.0
         << " "    << (real)(colB)/255.0
         << " setrgbcolor}bd\n";
//   fprintf(_psfh,"/tc_%s {%f %f %f setrgbcolor}bd\n", name.c_str(),
//                                                      (real)(colR)/255.0,
//                                                      (real)(colG)/255.0,
//                                                      (real)(colB)/255.0 );
}

void PsExportFile::defineFill(std::string pname, const byte* pat)
{
   _file << "<< /PatternType 1\n"
         << "   /PaintType 2\n"
         << "   /TilingType 1\n"
         << "   /BBox [0 0 32 32]\n"
         << "   /XStep 32\n"
         << "   /YStep 32\n"
         << "   /PaintProc\n"
         << "    { pop\n"
         << "      32 32\n"
         << "      true\n"
         << "      [1 0 0 1 0 0]\n"
         << "      {<";
   for(word i = 0; i < 32; i++)
   {
      if ((0 == i%4) && (i != 31))
         _file << "\n          ";
      char wstr[10];
      sprintf(wstr, "%02x%02x%02x%02x", pat[4*i+0], pat[4*i+1], pat[4*i+2], pat[4*i+3]);
      _file << wstr;
   }
   _file << "\n      >}\n"
         << "      imagemask\n"
         << "      fill\n"
         << "    } bind\n"
         << ">>\n"
         << "matrix\n"
         << "makepattern\n"
         << "/tp_" << pname <<" exch def\n"
         << "/dc_" << pname <<" {gsave dup ustroke currentrgbcolor "
         << "tp_"  << pname << " setpattern ufill grestore}bd\n";
}
