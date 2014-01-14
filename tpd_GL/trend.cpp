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
//        Created: Sun Sep  9 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Render dispatcher
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "trend.h"
#include <GL/glew.h>
#include "tolder.h"
#include "tenderer.h"
#include "toshader.h"
#include "viewprop.h"


trend::TrendCenter*              TRENDC  = NULL;
extern layprop::PropertyCenter*  PROPC;

//-----------------------------------------------------------------------------
// Initialize some static members
//-----------------------------------------------------------------------------
// All GLSL programs
//trend::GlslProgramIDs        trend::Shaders::_idPrograms;


//=============================================================================
//
//
//
trend::TGlfSymbol::TGlfSymbol(FILE* ffile)
{
   fread(&_alvrtxs, 1, 1, ffile);
   _vdata = DEBUG_NEW float[2 * _alvrtxs];
   fread(&_alchnks, 1, 1, ffile);
   _idata = DEBUG_NEW byte[3 * _alchnks];
   fread(&_alcntrs, 1, 1, ffile);
   _cdata = DEBUG_NEW byte[_alcntrs];
   // init the symbol boundaries
   _minX = _minY =  10;
   _maxX = _maxY = -10;
   // get the vertexes
   for (byte i = 0; i < _alvrtxs; i++)
   {
      float vX, vY;
      fread(&vX, 4, 1, ffile);
      fread(&vY, 4, 1, ffile);
      // update the symbol boundaries
      if      (vX < _minX) _minX = vX;
      else if (vX > _maxX) _maxX = vX;
      if      (vY < _minY) _minY = vY;
      else if (vY > _maxY) _maxY = vY;
      _vdata[2*i  ] = vX;
      _vdata[2*i+1] = vY;
   }
   // get index chunks
   for (byte i = 0; i < _alchnks; i++)
      fread(&(_idata[i*3]), 3, 1, ffile);
   // get contour data
   for (byte i = 0; i < _alcntrs; i++)
      fread(&(_cdata[i  ]), 1, 1, ffile);
}

void trend::TGlfSymbol::dataCopy(GLfloat* vxarray, GLuint* ixarray, word ioffset)
{
   memcpy(vxarray, _vdata, _alvrtxs * sizeof(float) * 2);
   for (word i = 0; i < _alchnks * 3; i++)
      ixarray[i] = _idata[i] + ioffset;
}

void trend::TGlfSymbol::draw(bool fill)
{
   if (fill)
   {
      glBegin(GL_TRIANGLES);
      for (byte i = 0; i < _alchnks; i++)
      {
         for (byte j = 0; j < 3;j++)
            glVertex2f(_vdata[2*_idata[3*i+j]],_vdata[2*_idata[3*i+j]+1]);
      }
      glEnd();
   }
   // Draw the contours
   word startIndex = 0;
   startIndex = 0;
   for (byte i = 0; i < _alcntrs; i++)
   {
      glBegin(GL_LINE_LOOP);
         for (byte j = startIndex; j < _cdata[i]+1; j++)
            glVertex2f(_vdata[2*j], _vdata[2*j+1]);
      glEnd();
      startIndex += _cdata[i]+1;
   }
}

trend::TGlfSymbol::~TGlfSymbol()
{
   delete [] _vdata;
   delete [] _idata;
   delete [] _cdata;
}

//=============================================================================
//
//
//
trend::TGlfRSymbol::TGlfRSymbol(TGlfSymbol* tsym, word voffset, word ioffset)
{
   _alcntrs = tsym->_alcntrs;
   _alchnks = tsym->_alchnks;
   //
   _csize = DEBUG_NEW GLsizei[_alcntrs];
   _firstvx = DEBUG_NEW GLint[_alcntrs];
   for (unsigned i = 0; i < _alcntrs; i++)
   {
      _csize[i] = tsym->_cdata[i] + 1;
      _firstvx[i] = voffset;
      if (0 != i)
      {
         _firstvx[i] += (tsym->_cdata[i-1] + 1);
         _csize[i]   -= (tsym->_cdata[i-1] + 1);
      }
   }
   _firstix = ioffset  * sizeof(unsigned);
   //
   _minX = tsym->_minX;
   _maxX = tsym->_maxX;
   _minY = tsym->_minY;
   _maxY = tsym->_maxY;
}

void trend::TGlfRSymbol::drawWired()
{
   glMultiDrawArrays(GL_LINE_LOOP, _firstvx, _csize, _alcntrs);
}

void trend::TGlfRSymbol::drawSolid()
{
   glDrawElements(GL_TRIANGLES, _alchnks * 3, GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(_firstix));
}

trend::TGlfRSymbol::~TGlfRSymbol()
{
   delete [] _firstvx;
   delete [] _csize;
}

//=============================================================================
//
//
//
trend::TolderGlfFont::TolderGlfFont(std::string filename, std::string& fontname) :
   _status    (             0    ),
   _pitch     (             0.1f ),
   _spaceWidth(             0.5f )
{
   FILE* ffile = fopen(filename.c_str(), "rb");
   if (NULL == ffile)
   {
      _status = 1;
      //@ TODO tellerror(...)
      return;
   }
   char fileh[4];
   fread(fileh, 3, 1, ffile);
   fileh[3] = 0x0;
   if (strcmp(fileh, "GLF"))
   {
      _status = 2;
      //@TODO tellerror(..."filename doesn't appear to be a font file");
   }
   else
   {
      // Get the font name
      char fname [97];
      fread(fname, 96, 1, ffile);
      fname[96] = 0x0;
      fontname = fname;
      // Get the layDef of symbols
      byte numSymbols;
      fread(&numSymbols, 1, 1, ffile);
      // Read the rest  of bytes to 128 (unused)
      byte unused [28];
      fread(unused, 28, 1, ffile);
      // Finally - start parsing the symbol data
      for (byte i = 0; i < numSymbols; i++)
      {
         byte asciiCode;
         fread(&asciiCode, 1, 1, ffile);
         TGlfSymbol* csymbol = DEBUG_NEW TGlfSymbol(ffile);
         _tsymbols[asciiCode] = csymbol;
      }
   }
   fclose(ffile);
}

void trend::TolderGlfFont::getStringBounds(const std::string& text, DBbox* overlap)
{
   // initialise the boundaries
   float top, bottom, left, right;
   if ((0x20 == text[0]) ||  (_tsymbols.end() == _tsymbols.find(text[0])))
   {
      left =   0.0f; right  = _spaceWidth;
      top  = -_spaceWidth; bottom = _spaceWidth;
   }
   else
   {
      left   = _tsymbols[text[0]]->minX();
      right  = _tsymbols[text[0]]->maxX();
      bottom = _tsymbols[text[0]]->minY();
      top    = _tsymbols[text[0]]->maxY();
   }
   // traverse the rest of the string
   for (unsigned i = 1; i < text.length() ; i++)
   {
      TFontMap::const_iterator CSI = _tsymbols.find(text[i]);
      if ((0x20 == text[i]) || (_tsymbols.end() == CSI))
         right += _spaceWidth;
      else
      {
         right += CSI->second->maxX() - CSI->second->minX() + _pitch;

         /* Update top/bottom bounds */
         if (CSI->second->minY() < bottom) bottom = CSI->second->minY();
         if (CSI->second->maxY() > top   ) top    = CSI->second->maxY();
      }
   }
   (*overlap) = DBbox(TP(left, bottom, OPENGL_FONT_UNIT), TP(right, top, OPENGL_FONT_UNIT));
}

void trend::TolderGlfFont::drawString(const std::string& text, bool fill, layprop::DrawProperties*)
{
   float right_of = 0.0f, left_of = 0.0f;
   for (unsigned i = 0; i < text.length() ; i++)
   {
      TFontMap::const_iterator CSI = _tsymbols.find(text[i]);
      if (i != 0)
      {
         // move one _pitch right
         if ((0x20 == text[i]) || (_tsymbols.end() == CSI))
            left_of = 0.0f;
         else
            left_of = -CSI->second->minX()+_pitch;
         glTranslatef(left_of+right_of, 0, 0);
      }
      if ((0x20 == text[i]) || (_tsymbols.end() == CSI))
      {
         glTranslatef(_spaceWidth, 0, 0);
         right_of = 0.0f;
      }
      else
      {
         CSI->second->draw(fill);
         right_of = CSI->second->maxX();
      }
   }
}

trend::TolderGlfFont::~TolderGlfFont()
{
   for (TFontMap::const_iterator CS = _tsymbols.begin(); CS != _tsymbols.end(); CS++)
      delete (CS->second);
}

//=============================================================================
//
//
//
trend::TenderGlfFont::TenderGlfFont(std::string filename, std::string& fontname) :
   TolderGlfFont   ( filename, fontname )
{
   if (0 == _status)
   {
      word all_vertexes = 0;
      word all_indexes = 0;
      for (TFontMap::const_iterator CS = _tsymbols.begin(); CS != _tsymbols.end(); CS++)
      {
         all_vertexes += CS->second->alvrtxs();
         all_indexes  += 3 * CS->second->alchnks(); // only triangles!
      }
      collect(all_vertexes, all_indexes);
   }
}

void trend::TenderGlfFont::collect(const word all_vertexes, const word all_indexes)
{
   // Create the VBO
   GLuint ogl_buffers[2];
   glGenBuffers(2, ogl_buffers);
   _pbuffer = ogl_buffers[0];
   _ibuffer = ogl_buffers[1];
   // Bind the buffers
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
   glBufferData(GL_ARRAY_BUFFER                   ,
                2 * all_vertexes * sizeof(float) ,
                NULL                              ,
                GL_STATIC_DRAW                     );
   float* cpoint_array = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER         ,
                all_indexes * sizeof(int)       ,
                NULL                            ,
                GL_STATIC_DRAW                   );
   GLuint* cindex_array = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

   //... and collect them deleting meanwhile the temporary objects
   word vrtx_indx = 0;
   word indx_indx = 0;
   for (TFontMap::const_iterator CRS = _tsymbols.begin(); CRS != _tsymbols.end(); CRS++)
   {
      TGlfRSymbol* csymbol = DEBUG_NEW TGlfRSymbol(CRS->second, vrtx_indx, indx_indx);
      CRS->second->dataCopy(&(cpoint_array[2 * vrtx_indx]), &(cindex_array[indx_indx]), vrtx_indx);
      vrtx_indx += CRS->second->alvrtxs();
      indx_indx += 3 * CRS->second->alchnks();
      _symbols[CRS->first] = csymbol;
      delete CRS->second;
   }
   _tsymbols.clear();
   assert(all_vertexes == vrtx_indx);
   assert(all_indexes  == indx_indx);
   glUnmapBuffer(GL_ARRAY_BUFFER);
   glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void trend::TenderGlfFont::bindBuffers()
{
   if ((0 ==_pbuffer) || (0 == _ibuffer)) return;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
//   GLint bufferSize;
//   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
//   bufferSize++;

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
//   glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
//   bufferSize++;
}

void trend::TenderGlfFont::getStringBounds(const std::string& text, DBbox* overlap)
{
   // initialise the boundaries
   float top, bottom, left, right;
   if ((0x20 == text[0]) ||  (_symbols.end() == _symbols.find(text[0])))
   {
      left =   0.0f; right  = _spaceWidth;
      top  = -_spaceWidth; bottom = _spaceWidth;
   }
   else
   {
      left   = _symbols[text[0]]->minX();
      right  = _symbols[text[0]]->maxX();
      bottom = _symbols[text[0]]->minY();
      top    = _symbols[text[0]]->maxY();
   }
   // traverse the rest of the string
   for (unsigned i = 1; i < text.length() ; i++)
   {
      FontMap::const_iterator CSI = _symbols.find(text[i]);
      if ((0x20 == text[i]) || (_symbols.end() == CSI))
         right += _spaceWidth;
      else
      {
         right += CSI->second->maxX() - CSI->second->minX() + _pitch;

         /* Update top/bottom bounds */
         if (CSI->second->minY() < bottom) bottom = CSI->second->minY();
         if (CSI->second->maxY() > top   ) top    = CSI->second->maxY();
      }
   }
   (*overlap) = DBbox(TP(left, bottom, OPENGL_FONT_UNIT), TP(right, top, OPENGL_FONT_UNIT));
}

void trend::TenderGlfFont::drawString(const std::string& text, bool fill, layprop::DrawProperties*)
{
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, 0, NULL);
   if (fill)
      glEnableClientState(GL_INDEX_ARRAY);
   float right_of = 0.0f, left_of = 0.0f;
   for (unsigned i = 0; i < text.length() ; i++)
   {
      FontMap::const_iterator CSI = _symbols.find(text[i]);
      if (i != 0)
      {
         // move one _pitch right
         if ((0x20 == text[i]) || (_symbols.end() == CSI))
            left_of = 0.0f;
         else
            left_of = -CSI->second->minX()+_pitch;
         glTranslatef(left_of+right_of, 0, 0);
      }
      if ((0x20 == text[i]) || (_symbols.end() == CSI))
      {
         glTranslatef(_spaceWidth, 0, 0);
         right_of = 0.0f;
      }
      else
      {
         CSI->second->drawWired();
         if (fill)
            CSI->second->drawSolid();
         right_of = CSI->second->maxX();
      }
   }
   if (fill)
      glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
}

trend::TenderGlfFont::~TenderGlfFont()
{
   for (FontMap::const_iterator CS = _symbols.begin(); CS != _symbols.end(); CS++)
      delete (CS->second);
   GLuint ogl_buffers[2] = {_pbuffer, _ibuffer};
   glDeleteBuffers(2, ogl_buffers);
}

//=============================================================================
trend::ToshaderGlfFont::ToshaderGlfFont(std::string filename, std::string& fontname) :
   TenderGlfFont(filename, fontname)
{}

void trend::ToshaderGlfFont::drawString(const std::string& text, bool fill, layprop::DrawProperties* drawprop)
{
   // Activate the vertex buffers in the vertex shader ...
   glEnableVertexAttribArray(TSHDR_LOC_VERTEX);
   // Set-up the offset in the binded Vertex buffer
   glVertexAttribPointer(TSHDR_LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
   float right_of = 0.0f, left_of = 0.0f;
   for (unsigned i = 0; i < text.length() ; i++)
   {
      FontMap::const_iterator CSI = _symbols.find(text[i]);
      if (i != 0)
      {
         // move one _pitch right
         if (!((0x20 == text[i]) || (_symbols.end() == CSI)))
            left_of += -CSI->second->minX()+_pitch;

         CTM ctm;
         ctm.Translate(left_of+right_of, 0);
         ctm *= drawprop->topCtm();
         float mtrxOrtho[16];
         ctm.oglForm(mtrxOrtho);
         glUniformMatrix4fv(TRENDC->getUniformLoc(glslu_in_CTM), 1, GL_FALSE, mtrxOrtho);
      }
      if ((0x20 == text[i]) || (_symbols.end() == CSI))
      {
         right_of += _spaceWidth;
      }
      else
      {
         glUniform1ui(TRENDC->getUniformLoc(glslu_in_StippleEn), 0);
         CSI->second->drawWired();
         glUniform1ui(TRENDC->getUniformLoc(glslu_in_StippleEn), 1);
         if (fill)
            CSI->second->drawSolid();
         right_of += CSI->second->maxX();
      }
   }
   glDisableVertexAttribArray(TSHDR_LOC_VERTEX);
}



//=============================================================================
trend::Shaders::Shaders() :
   _fnShdrVertex     ( "vertex.glsl"      ),
   _fnShdrGeometry   ( "geometry.glsl"    ),
   _fnShdrGeSprite   ( "geomsprite.glsl"  ),
   _fnShdrFragment   ( "fragment.glsl"    ),
   _idShdrVertex     ( -1                 ),
   _idShdrGeometry   ( -1                 ),
   _idShdrGeSprite   ( -1                 ),
   _idShdrFragment   ( -1                 ),
   _curProgram       ( glslp_NULL         ),
   _status           ( true               )
{
   // initialize all uniform variable names
   _glslUniVarNames[glslp_VF][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_VF][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_VF][glslu_in_Color]      = "in_Color";
   _glslUniVarNames[glslp_VF][glslu_in_Alpha]      = "in_Alpha";
   _glslUniVarNames[glslp_VF][glslu_in_Stipple]    = "in_Stipple";
   _glslUniVarNames[glslp_VF][glslu_in_LStipple]   = "in_LStipple";
   _glslUniVarNames[glslp_VF][glslu_in_StippleEn]  = "in_StippleEn";
   _glslUniVarNames[glslp_VF][glslu_in_LStippleEn] = "in_LStippleEn";
   _glslUniVarNames[glslp_VF][glslu_in_MStippleEn] = "in_MStippleEn";

   _glslUniVarNames[glslp_VG][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_VG][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_VG][glslu_in_Color]      = "in_Color";
   _glslUniVarNames[glslp_VG][glslu_in_Alpha]      = "in_Alpha";
   _glslUniVarNames[glslp_VG][glslu_in_Stipple]    = "in_Stipple";
   _glslUniVarNames[glslp_VG][glslu_in_LStipple]   = "in_LStipple";
   _glslUniVarNames[glslp_VG][glslu_in_StippleEn]  = "in_StippleEn";
   _glslUniVarNames[glslp_VG][glslu_in_LStippleEn] = "in_LStippleEn";
   _glslUniVarNames[glslp_VG][glslu_in_ScreenSize] = "in_ScreenSize";
   _glslUniVarNames[glslp_VG][glslu_in_PatScale]   = "in_PatScale";
   _glslUniVarNames[glslp_VG][glslu_in_MStippleEn] = "in_MStippleEn";

   _glslUniVarNames[glslp_PS][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_PS][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_PS][glslu_in_Color]      = "in_Color";
   _glslUniVarNames[glslp_PS][glslu_in_Alpha]      = "in_Alpha";
   _glslUniVarNames[glslp_PS][glslu_in_Stipple]    = "in_Stipple";
   _glslUniVarNames[glslp_PS][glslu_in_LStipple]   = "in_LStipple";
   _glslUniVarNames[glslp_PS][glslu_in_StippleEn]  = "in_StippleEn";
   _glslUniVarNames[glslp_PS][glslu_in_LStippleEn] = "in_LStippleEn";
   _glslUniVarNames[glslp_PS][glslu_in_ScreenSize] = "in_ScreenSize";
   _glslUniVarNames[glslp_PS][glslu_in_MStippleEn] = "in_MStippleEn";
   //
   _idPrograms[glslp_VF] = -1;
   _idPrograms[glslp_VG] = -1;
   _idPrograms[glslp_PS] = -1;
}

void trend::Shaders::useProgram(const glsl_Programs pType)
{
   _curProgram = pType;
   assert(-1 != _idPrograms[_curProgram]);
   glUseProgram(_idPrograms[pType]);
   if (  (glslp_VG == _curProgram)
       ||(glslp_PS == _curProgram)
       )
   {
      // view port
      GLfloat vport[4];
      glGetFloatv(GL_VIEWPORT, vport);
      glUniform2fv(getUniformLoc(glslu_in_ScreenSize), 1, &vport[2]);
   }
}

GLint trend::Shaders::getUniformLoc(const glsl_Uniforms var) const
{
   assert((_idPrograms.end() != _idPrograms.find(_curProgram)) &&
                         (-1 != (_idPrograms.find(_curProgram))->second));
   GlslUniVarAllLoc::const_iterator varSet = _glslUniVarLoc.find(_curProgram);
   assert(_glslUniVarLoc.end() != varSet);
   GlslUniVarLoc::const_iterator varLoc    = varSet->second.find(var);
   assert(varSet->second.end() != varLoc);
   return varLoc->second;
}

void trend::Shaders::loadShadersCode(const std::string& codeDirectory)
{
   _fnShdrVertex   = codeDirectory + _fnShdrVertex;
   _fnShdrGeometry = codeDirectory + _fnShdrGeometry;
   _fnShdrGeSprite = codeDirectory + _fnShdrGeSprite;
   _fnShdrFragment = codeDirectory + _fnShdrFragment;
   if(  (_status &= compileShader(_fnShdrVertex  , _idShdrVertex  , GL_VERTEX_SHADER  ))
      &&(_status &= compileShader(_fnShdrGeometry, _idShdrGeometry, GL_GEOMETRY_SHADER))
      &&(_status &= compileShader(_fnShdrGeSprite, _idShdrGeSprite, GL_GEOMETRY_SHADER))
      &&(_status &= compileShader(_fnShdrFragment, _idShdrFragment, GL_FRAGMENT_SHADER))
     )
   {
      _status &= linkProgram(glslp_VF);
      _status &= linkProgram(glslp_VG);
      _status &= linkProgram(glslp_PS);
   }
}

bool trend::Shaders::linkProgram(const glsl_Programs pType)
{
   GLint program = glCreateProgram();
   if (0 == program)
   {
      return false;
   }

   // TODO bind attribute locations

   std::stringstream info;
   switch (pType)
   {
      case glslp_VF:
      {
         glAttachShader(program, _idShdrVertex  );
         glAttachShader(program, _idShdrFragment);
         info << "GLSL program VF";
         break;
      }
      case glslp_VG:
      {
         glAttachShader(program, _idShdrVertex  );
         glAttachShader(program, _idShdrGeometry);
         glAttachShader(program, _idShdrFragment);
         info << "GLSL program VG";
         break;
      }
      case glslp_PS:
      {
         glAttachShader(program, _idShdrVertex  );
         glAttachShader(program, _idShdrGeSprite);
         glAttachShader(program, _idShdrFragment);
         info << "GLSL program PS";
         break;
      }
      default: assert(false); break;
   }
   // link
   glLinkProgram(program);
   GLint programOK;
   glGetProgramiv(program, GL_LINK_STATUS, &programOK);
   if (programOK)
   {
      info << " linked";
      tell_log(console::MT_INFO, info.str());
      _idPrograms[pType] = program;
      return bindUniforms(pType);
   }
   else
   {
      info << " link failed. See the log below:";
      tell_log(console::MT_ERROR, info.str());
      getProgramsLog(program);
      return false;
   }
}

bool trend::Shaders::bindUniforms(const glsl_Programs pType)
{
   for (GlslUniVarNames::const_iterator CV = _glslUniVarNames[pType].begin(); CV != _glslUniVarNames[pType].end(); CV++)
   {
      GLint location = glGetUniformLocation(_idPrograms[pType], CV->second.c_str());
      if( location >= 0 )
      {
         _glslUniVarLoc[pType][CV->first] = location;
      }
      else
      {
         std::stringstream info;
         info << "Can't bind variable \"" << CV->second << "\"";
         tell_log(console::MT_ERROR, info.str());
         return false;
      }
   }
   return true;
}

bool trend::Shaders::compileShader(const std::string& fname, GLint& idShader, GLint sType)
{
   GLint allLenShaders;
   const GLchar* allStrShaders = loadFile(fname, allLenShaders);
   if (0 != allLenShaders)
   {
      idShader = glCreateShader( sType );
      glShaderSource(idShader, 1, &allStrShaders, &allLenShaders);
      glCompileShader(idShader);
      GLint statusOK;
      glGetShaderiv(idShader, GL_COMPILE_STATUS, &statusOK);
      if (statusOK)
      {
         std::stringstream info;
         info << "Shader " << fname << " compiled";
         tell_log(console::MT_INFO, info.str());
      }
      else
      {
         std::stringstream info;
         info << "Shader " << fname << " failed to compile. See the log below:";
         tell_log(console::MT_ERROR, info.str());
         getShadersLog(idShader);
      }
      delete [] allStrShaders;
      return (GL_TRUE == statusOK);
   }
   else
      return false;
}

char* trend::Shaders::loadFile(const std::string& fName, GLint& fSize)
{
   std::ifstream file(fName.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
   if (file.is_open())
   {
      std::ifstream::pos_type size = file.tellg();
      fSize = (GLint) size;
      char* memBlock = DEBUG_NEW char [size];
      file.seekg(0, std::ios::beg);
      file.read(memBlock, size);
      file.close();
      return memBlock;
   }
   else
   {
      std::stringstream info;
      info << "Can't load the shader file \"" << fName << "\"";
      tell_log(console::MT_ERROR, info.str());
      fSize = 0;
      return NULL;
   }
}

void trend::Shaders::getShadersLog(GLint idShader)
{
   int      lenInfoLog = 0;
   int      lenFetched = 0;
   GLchar*  infoLog    = NULL;

   glGetShaderiv(idShader, GL_INFO_LOG_LENGTH, &lenInfoLog);

   if (lenInfoLog > 0)
   {
      infoLog = DEBUG_NEW GLchar[lenInfoLog];
      glGetShaderInfoLog(idShader, lenInfoLog, &lenFetched, infoLog);
      tell_log(console::MT_INFO,infoLog);
      delete [] infoLog;
   }
}

void trend::Shaders::getProgramsLog(GLint idProgram)
{
   int      lenInfoLog = 0;
   int      lenFetched = 0;
   GLchar*  infoLog    = NULL;

   glGetProgramiv(idProgram, GL_INFO_LOG_LENGTH, &lenInfoLog);

   if (lenInfoLog > 0)
   {
      infoLog = DEBUG_NEW GLchar[lenInfoLog];
      glGetProgramInfoLog(idProgram, lenInfoLog, &lenFetched, infoLog);
      tell_log(console::MT_INFO,infoLog);
      delete [] infoLog;
   }
}

trend::Shaders::~Shaders()
{
}

//=============================================================================
trend::TrendCenter::TrendCenter(bool gui, RenderType cmdLineReq, bool sprtVbo, bool sprtShaders) :
   _cRenderer       (              NULL),
   _hRenderer       (              NULL),
   _mRenderer       (              NULL),
   _zRenderer       (              NULL),
   _cShaders        (              NULL),
   _activeFontName  (                  )

{
   if      (!gui)             _renderType = trend::rtTocom;
   else if ( sprtShaders)     _renderType = trend::rtToshader;
   else if ( sprtVbo    )     _renderType = trend::rtTenderer;
   else                       _renderType = trend::rtTolder;
   if ((cmdLineReq != trend::rtTBD) & (cmdLineReq < _renderType))
      _renderType = cmdLineReq;
   if (trend::rtToshader== _renderType)
      _cShaders = DEBUG_NEW trend::Shaders();
}

void trend::TrendCenter::reportRenderer(RenderType cmdLineReq) const
{
   switch (cmdLineReq)
   {
      case trend::rtTolder  :
      {
         if (cmdLineReq > _renderType)
            tell_log(console::MT_WARNING, "Platform dosn't support basic rendering as requested on the command line. Using text mode.");
         else
            tell_log(console::MT_INFO, "Basic rendering enforced from the command line.");
         break;
      }
      case trend::rtTenderer:
      {
         if (cmdLineReq > _renderType)
         {
            std::string info = "Platform dosn't support VBO rendering as requested on the command line.";
            if      (trend::rtTolder == _renderType)
               info += "Using basic rendering.";
            else if (trend::rtTocom  == _renderType)
               info += "Using text mode.";
            else
               assert(false);
            tell_log(console::MT_WARNING, info);
         }
         else
            tell_log(console::MT_INFO, "VBO rendering enforced from the command line.");
         break;
      }
      case trend::rtToshader:
      {
         if (cmdLineReq > _renderType)
         {
            std::string info = "Platform dosn't support shaders as requested on the command line.";
            if      (trend::rtTenderer == _renderType)
               info += "Using VBO rendering.";
            else if (trend::rtTolder == _renderType)
               info += "Using basic rendering.";
            else if (trend::rtTocom  == _renderType)
               info += "Using text mode.";
            else
               assert(false);
            tell_log(console::MT_WARNING, info);
         }
         else
            tell_log(console::MT_INFO, "Shader rendering enforced from the command line.");
         break;
      }
      case trend::rtTBD     : {
         switch (_renderType)
         {
            case trend::rtTocom   : /* Don't clutter the command line with nonsence*/ break;
            case trend::rtTolder  : tell_log(console::MT_INFO,"Using basic rendering."); break;
            case trend::rtTenderer: tell_log(console::MT_INFO,"Using VBO rendering.");break;
            case trend::rtToshader: tell_log(console::MT_INFO,"Using shader rendering.");break;
            default: assert(false); break;
         }
         break;
      }
      case trend::rtTocom   : assert(false); /*if you hit this and -nogui cmdline option has been removed, remove the assert*/ break;
      default: assert(false); break;
   }
}

void trend::TrendCenter::initShaders(const std::string& codeDirectory)
{
   if (trend::rtToshader == _renderType)
   {
      assert(_cShaders);
      _cShaders->loadShadersCode(codeDirectory);
      if (_cShaders->status())
      {
         _cShaders->useProgram(glslp_VF);
      }
      else
      {
         tell_log(console::MT_WARNING, "Falling back to VBO rendering because of the errors above");
         _renderType = trend::rtTenderer;
      }
   }
}

trend::TrendBase* trend::TrendCenter::getCRenderer()
{
   // Don't block the drawing if the databases are
   // locked. This will block all redraw activities including UI
   // which have nothing to do with the DB. Drop a message in the log
   // and keep going!
   assert(NULL == _cRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp))
   {
      switch (_renderType)
      {
         case trend::rtTocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::rtTolder   :
            _cRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() );break;
         case trend::rtTenderer :
            _cRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::rtToshader : 
            _cRenderer = DEBUG_NEW trend::Toshader( drawProp, PROPC->UU() ); break;
         default: assert(false); break;
      }
   }
   else
   {
      // If property DB is locked - we can't do much drawing even if the
      // DB is not locked. In the same time there should not be an operation
      // which holds the property DB lock for a long time. So it should be
      // rather an exception
      tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
   }
   return _cRenderer;
}

trend::TrendBase* trend::TrendCenter::getHRenderer()
{
   assert(NULL == _hRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp))
   {
      switch (_renderType)
      {
         case trend::rtTocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::rtTolder   :
            _hRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() ); break;
         case trend::rtTenderer :
            _hRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::rtToshader :
            _hRenderer = DEBUG_NEW trend::Toshader( drawProp, PROPC->UU() ); break;
         default: assert(false); break;
      }
   }
   return _hRenderer;
}

trend::TrendBase* trend::TrendCenter::getMRenderer(console::ACTIVE_OP& curOp)
{
   assert(NULL == _mRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp))
   {
      switch (_renderType)
      {
         case trend::rtTocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::rtTolder   :
            _mRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() ); break;
         case trend::rtTenderer :
            _mRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::rtToshader :
            _mRenderer = DEBUG_NEW trend::Toshader( drawProp, PROPC->UU() ); break;
         default: assert(false); break;
      }
      curOp = drawProp->currentOp();
   }
   return _mRenderer;
}

trend::TrendBase* trend::TrendCenter::getZRenderer()
{
   assert(NULL == _zRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp, layprop::prsSCR))
   {
      switch (_renderType)
      {
      case trend::rtTocom: assert(false);          break;// shouldn't end-up here ever
      case trend::rtTolder:
         _zRenderer = DEBUG_NEW trend::Tolder(drawProp, PROPC->UU()); break;
      case trend::rtTenderer:
         _zRenderer = DEBUG_NEW trend::Tenderer(drawProp, PROPC->UU()); break;
      case trend::rtToshader:
         _zRenderer = DEBUG_NEW trend::Toshader(drawProp, PROPC->UU()); break;
      default: assert(false); break;
      }
   }
   return _zRenderer;
}

void trend::TrendCenter::releaseCRenderer()
{
   assert(NULL != _cRenderer);
   PROPC->unlockDrawProp(_cRenderer->drawprop(), false);
   delete (_cRenderer);
   _cRenderer = NULL;
}

void trend::TrendCenter::releaseHRenderer()
{
   assert(NULL != _hRenderer);
   PROPC->unlockDrawProp(_hRenderer->drawprop(), false);
   delete (_hRenderer);
   _hRenderer = NULL;
}

void trend::TrendCenter::releaseMRenderer()
{
   assert(NULL != _mRenderer);
   PROPC->unlockDrawProp(_mRenderer->drawprop(), false);
   delete (_mRenderer);
   _mRenderer = NULL;
}

void trend::TrendCenter::releaseZRenderer()
{
   assert(NULL != _zRenderer);
   PROPC->unlockDrawProp(_zRenderer->drawprop(), false);
   delete (_zRenderer);
   _zRenderer = NULL;
}

void trend::TrendCenter::drawFOnly()
{
//   if (NULL != _cRenderer) _cRenderer->grcDraw(); // TODO
}


void trend::TrendCenter::loadLayoutFont(std::string fontfile)
{
   // Parse the font library
   TolderGlfFont* curFont = NULL;
   switch (_renderType)
   {
      case trend::rtTocom    : assert(false);          break;// TODO?
      case trend::rtTolder   :
         curFont = DEBUG_NEW trend::TolderGlfFont(fontfile, _activeFontName);
         break;
      case trend::rtTenderer :
         curFont = DEBUG_NEW trend::TenderGlfFont(fontfile, _activeFontName);
         break;
      case trend::rtToshader :
         curFont = DEBUG_NEW trend::ToshaderGlfFont(fontfile, _activeFontName);
         break;
      default: assert(false); break;
   }

   if (!curFont->status())
   {
      _oglFont[_activeFontName] = curFont;
      TpdPost::addFont(_activeFontName);
   }
}

void trend::TrendCenter::getStringBounds(const std::string& text, DBbox* overlap)
{
   assert(NULL != _oglFont[_activeFontName]); // make sure that fonts are initialised
   _oglFont[_activeFontName]->getStringBounds(text, overlap);

}

void trend::TrendCenter::drawString(const std::string& text, bool fill, layprop::DrawProperties* drawprop)
{
   _oglFont[_activeFontName]->drawString(text, fill, drawprop);
}

//void trend::TrendCenter::drawWiredString(const std::string& text)
//{
//   bindFont();
//   _oglFont[_activeFontName]->drawString(text, false,NULL);
//   unbindFont();
//
//}
//
//void trend::TrendCenter::drawSolidString(const std::string& text)
//{
//   bindFont();
//   _oglFont[_activeFontName]->drawString(text, true, NULL);
//   unbindFont();
//}

bool trend::TrendCenter::selectFont(std::string fname)
{
   if (_oglFont.end() != _oglFont.find(fname))
   {
      _activeFontName = fname;
      return true;
   }
   return false;
}

word trend::TrendCenter::numFonts()
{
   return _oglFont.size();

}

void trend::TrendCenter::bindFont()
{
   if (NULL != _oglFont[_activeFontName])
      _oglFont[_activeFontName]->bindBuffers();

}

void trend::TrendCenter::unbindFont()
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLint trend::TrendCenter::getUniformLoc(const glsl_Uniforms var) const
{
   assert(_cShaders);
   return _cShaders->getUniformLoc(var);
}

void trend::TrendCenter::setGlslProg(const glsl_Programs prog) const
{
   assert(_cShaders);
   _cShaders->useProgram(prog);
}

trend::TrendCenter::~TrendCenter()
{
   for (OglFontCollectionMap::const_iterator CF = _oglFont.begin(); CF != _oglFont.end(); CF++)
      delete (CF->second);
   if (NULL != _cRenderer) delete _cRenderer;
   if (NULL != _cShaders) delete _cShaders;
}

