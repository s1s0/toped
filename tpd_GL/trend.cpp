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
      DBGL_CALL(glBegin,GL_TRIANGLES)
      for (byte i = 0; i < _alchnks; i++)
      {
         for (byte j = 0; j < 3;j++)
            glVertex2f(_vdata[2*_idata[3*i+j]],_vdata[2*_idata[3*i+j]+1]);
      }
      DBGL_CALL0(glEnd)
   }
   // Draw the contours
   word startIndex = 0;
   startIndex = 0;
   for (byte i = 0; i < _alcntrs; i++)
   {
      DBGL_CALL(glBegin,GL_LINE_LOOP)
         for (byte j = startIndex; j < _cdata[i]+1; j++)
            glVertex2f(_vdata[2*j], _vdata[2*j+1]);
      DBGL_CALL0(glEnd)
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
   DBGL_CALL(glMultiDrawArrays,GL_LINE_LOOP, _firstvx, _csize, _alcntrs)
}

void trend::TGlfRSymbol::drawSolid()
{
   DBGL_CALL(tpd_glDrawElements,GL_TRIANGLES, _alchnks * 3, GL_UNSIGNED_INT, _firstix)
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
   DBGL_CALL(glGenBuffers, 2, ogl_buffers)
   _pbuffer = ogl_buffers[0];
   _ibuffer = ogl_buffers[1];
   // Bind the buffers
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER,_pbuffer)
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER                  ,
                                  2 * all_vertexes * sizeof(float) ,
                                  nullptr                          ,
                                  GL_STATIC_DRAW )
   float* cpoint_array = (float*)DBGL_CALL(glMapBuffer,GL_ARRAY_BUFFER, GL_WRITE_ONLY)

   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
   DBGL_CALL(glBufferData ,GL_ELEMENT_ARRAY_BUFFER                ,
                                  all_indexes * sizeof(int)       ,
                                  nullptr                         ,
                                  GL_STATIC_DRAW                   )
   GLuint* cindex_array = (GLuint*)DBGL_CALL(glMapBuffer,GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)

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
   DBGL_CALL(glUnmapBuffer,GL_ARRAY_BUFFER)
   DBGL_CALL(glUnmapBuffer,GL_ELEMENT_ARRAY_BUFFER)
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
}

void trend::TenderGlfFont::bindBuffers()
{
   if ((0 ==_pbuffer) || (0 == _ibuffer)) return;
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, _pbuffer)
//   GLint bufferSize;
//   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
//   bufferSize++;

   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, _ibuffer)
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
   DBGL_CALL(glDeleteBuffers,2, ogl_buffers)
}

//=============================================================================
trend::ToshaderGlfFont::ToshaderGlfFont(std::string filename, std::string& fontname) :
   TenderGlfFont(filename, fontname)
{}

void trend::ToshaderGlfFont::drawString(const std::string& text, bool fill, layprop::DrawProperties* drawprop)
{
   // Activate the vertex buffers in the vertex shader ...
   DBGL_CALL(glEnableVertexAttribArray,TSHDR_LOC_VERTEX)
   // Set-up the offset in the binded Vertex buffer
   DBGL_CALL(glVertexAttribPointer, TSHDR_LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr)
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
         TRENDC->setUniMtrx4fv(glslu_in_CTM, mtrxOrtho);
      }
      if ((0x20 == text[i]) || (_symbols.end() == CSI))
      {
         right_of += _spaceWidth;
      }
      else
      {
         TRENDC->setUniVarui(glslu_in_StippleEn, 0);
         CSI->second->drawWired();
         TRENDC->setUniVarui(glslu_in_StippleEn, 1);
         if (fill)
            CSI->second->drawSolid();
         right_of += CSI->second->maxX();
      }
   }
   DBGL_CALL(glDisableVertexAttribArray, TSHDR_LOC_VERTEX)
}



//=============================================================================
trend::Shaders::Shaders() :
   _fnShdrVertex     ( "vertex.glsl"      ),
   _fnShdrGeometry   ( "geometry.glsl"    ),
   _fnShdrGeSprite   ( "geomsprite.glsl"  ),
   _fnShdrFragment   ( "fragment.glsl"    ),
   _fnShdrFBVertex   ( "vertex_fb.glsl"   ),
   _fnShdrFBFragment ( "fragment_fb.glsl" ),
   _idShdrVertex     ( -1                 ),
   _idShdrGeometry   ( -1                 ),
   _idShdrGeSprite   ( -1                 ),
   _idShdrFragment   ( -1                 ),
   _curProgram       ( glslp_NULL         ),
   _status           ( true               ),
   _fbProps          ({0,0,0,0,0}         )
{
   // initialize all uniform variable names
   _glslUniVarNames[glslp_VF][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_VF][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_VF][glslu_in_Color]      = "in_Color";
//   _glslUniVarNames[glslp_VF][glslu_in_Alpha]      = "in_Alpha";
   _glslUniVarNames[glslp_VF][glslu_in_Stipple]    = "in_Stipple";
   _glslUniVarNames[glslp_VF][glslu_in_LStipple]   = "in_LStipple";
   _glslUniVarNames[glslp_VF][glslu_in_StippleEn]  = "in_StippleEn";
   _glslUniVarNames[glslp_VF][glslu_in_LStippleEn] = "in_LStippleEn";
   _glslUniVarNames[glslp_VF][glslu_in_MStippleEn] = "in_MStippleEn";

   _glslUniVarNames[glslp_VG][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_VG][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_VG][glslu_in_Color]      = "in_Color";
//   _glslUniVarNames[glslp_VG][glslu_in_Alpha]      = "in_Alpha";
   _glslUniVarNames[glslp_VG][glslu_in_Stipple]    = "in_Stipple";
   _glslUniVarNames[glslp_VG][glslu_in_LStipple]   = "in_LStipple";
   _glslUniVarNames[glslp_VG][glslu_in_LWidth]     = "in_LWidth";
   _glslUniVarNames[glslp_VG][glslu_in_StippleEn]  = "in_StippleEn";
   _glslUniVarNames[glslp_VG][glslu_in_LStippleEn] = "in_LStippleEn";
   _glslUniVarNames[glslp_VG][glslu_in_ScreenSize] = "in_ScreenSize";
   _glslUniVarNames[glslp_VG][glslu_in_PatScale]   = "in_PatScale";
   _glslUniVarNames[glslp_VG][glslu_in_MStippleEn] = "in_MStippleEn";

   _glslUniVarNames[glslp_PS][glslu_in_CTM]        = "in_CTM";
   _glslUniVarNames[glslp_PS][glslu_in_Z]          = "in_Z";
   _glslUniVarNames[glslp_PS][glslu_in_Color]      = "in_Color";
//   _glslUniVarNames[glslp_PS][glslu_in_Alpha]      = "in_Alpha";
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
   DBGL_CALL(glUseProgram, _idPrograms[pType])
   if (  (glslp_VG == _curProgram)
       ||(glslp_PS == _curProgram)
       )
   {
      // view port
      GLfloat vport[4];
      DBGL_CALL(glGetFloatv,GL_VIEWPORT, vport)
      DBGL_CALL(glUniform2fv, getUniformLoc(glslu_in_ScreenSize), 1, &vport[2])
      if (glslp_VG == _curProgram)
         setUniVarf(glslu_in_LWidth, 1);
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

void trend::Shaders::setUniVarf(const glsl_Uniforms varName, GLfloat varValue) const
{
   DBGL_CALL(glUniform1f, getUniformLoc(varName), varValue)
}

void trend::Shaders::setUniMtrx4fv(const glsl_Uniforms varName, GLfloat* varValue) const
{
   DBGL_CALL(glUniformMatrix4fv,getUniformLoc(varName), 1, GL_FALSE, varValue)
}

void trend::Shaders::setUniColor(/*const glsl_Uniforms varName, */GLfloat* varValue) const
{
   DBGL_CALL(glUniform4fv,getUniformLoc(glslu_in_Color), 1, varValue)
}

void trend::Shaders::setUniVarui(const glsl_Uniforms varName, GLuint varValue) const
{
   DBGL_CALL(glUniform1ui, getUniformLoc(varName), varValue)
}

void trend::Shaders::setUniStipple(/*const glsl_Uniforms varName,*/ GLuint* varValue) const
{
   DBGL_CALL(glUniform1uiv, getUniformLoc(glslu_in_Stipple), 33, varValue)
}

void trend::Shaders::loadShadersCode(const std::string& codeDirectory)
{
   _fnShdrVertex     = codeDirectory + _fnShdrVertex    ;
   _fnShdrGeometry   = codeDirectory + _fnShdrGeometry  ;
   _fnShdrGeSprite   = codeDirectory + _fnShdrGeSprite  ;
   _fnShdrFragment   = codeDirectory + _fnShdrFragment  ;
   _fnShdrFBVertex   = codeDirectory + _fnShdrFBVertex  ;
   _fnShdrFBFragment = codeDirectory + _fnShdrFBFragment;
   if(  (_status &= compileShader(_fnShdrVertex    , _idShdrVertex    , GL_VERTEX_SHADER   ))
      &&(_status &= compileShader(_fnShdrGeometry  , _idShdrGeometry  , GL_GEOMETRY_SHADER ))
      &&(_status &= compileShader(_fnShdrGeSprite  , _idShdrGeSprite  , GL_GEOMETRY_SHADER ))
      &&(_status &= compileShader(_fnShdrFragment  , _idShdrFragment  , GL_FRAGMENT_SHADER ))
      &&(_status &= compileShader(_fnShdrFBVertex  , _idShdrFBVertex  , GL_VERTEX_SHADER   ))
      &&(_status &= compileShader(_fnShdrFBFragment, _idShdrFBFragment, GL_FRAGMENT_SHADER ))
     )
   {
      _status &= linkProgram(glslp_VF);
      _status &= linkProgram(glslp_VG);
      _status &= linkProgram(glslp_PS);
      _status &= linkProgram(glslp_FB);
      
      DBGL_CALL(glDeleteShader,_idShdrVertex    )
      DBGL_CALL(glDeleteShader,_idShdrGeometry  )
      DBGL_CALL(glDeleteShader,_idShdrGeSprite  )
      DBGL_CALL(glDeleteShader,_idShdrFragment  )
      DBGL_CALL(glDeleteShader,_idShdrFBVertex  )
      DBGL_CALL(glDeleteShader,_idShdrFBFragment)
      
   }
}

bool trend::Shaders::linkProgram(const glsl_Programs pType)
{
   GLint program = DBGL_CALL0(glCreateProgram)
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
         DBGL_CALL(glAttachShader, program, _idShdrVertex  )
         DBGL_CALL(glAttachShader, program, _idShdrFragment)
         info << "GLSL program VF";
         break;
      }
      case glslp_VG:
      {
         DBGL_CALL(glAttachShader, program, _idShdrVertex  )
         DBGL_CALL(glAttachShader, program, _idShdrGeometry)
         DBGL_CALL(glAttachShader, program, _idShdrFragment)
         info << "GLSL program VG";
         break;
      }
      case glslp_PS:
      {
         DBGL_CALL(glAttachShader, program, _idShdrVertex  )
         DBGL_CALL(glAttachShader, program, _idShdrGeSprite)
         DBGL_CALL(glAttachShader, program, _idShdrFragment)
         info << "GLSL program PS";
         break;
      }
      case glslp_FB:
      {
         DBGL_CALL(glAttachShader, program, _idShdrFBVertex   )
         DBGL_CALL(glAttachShader, program, _idShdrFBFragment )
         info << "GLSL program FB";
         break;
      }
      default: assert(false); break;
   }
   // link
   DBGL_CALL(glLinkProgram,program)
   GLint programOK;
   DBGL_CALL(glGetProgramiv,program, GL_LINK_STATUS, &programOK)
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
   
   switch (pType)
   {
      case glslp_VF:
      {
         DBGL_CALL(glDetachShader, program, _idShdrVertex  )
         DBGL_CALL(glDetachShader, program, _idShdrFragment)
         break;
      }
      case glslp_VG:
      {
         DBGL_CALL(glDetachShader, program, _idShdrVertex  )
         DBGL_CALL(glDetachShader, program, _idShdrGeometry)
         DBGL_CALL(glDetachShader, program, _idShdrFragment)
         break;
      }
      case glslp_PS:
      {
         DBGL_CALL(glDetachShader, program, _idShdrVertex  )
         DBGL_CALL(glDetachShader, program, _idShdrGeSprite)
         DBGL_CALL(glDetachShader, program, _idShdrFragment)
         break;
      }
      default: assert(false); break;
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

bool trend::Shaders::setFrameBuffer(int W, int H)
{
   clearFrameBuffer();

   DBGL_CALL(glGenFramebuffers, 1, &_fbProps.FBO)
   DBGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, _fbProps.FBO)
   // create a color attachment texture
   DBGL_CALL(glGenTextures, 1, &_fbProps.texture)
   DBGL_CALL(glBindTexture, GL_TEXTURE_2D, _fbProps.texture)
   DBGL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr)
   DBGL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
   DBGL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
   DBGL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fbProps.texture, 0)
   // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
   DBGL_CALL(glGenRenderbuffers, 1, &_fbProps.RBO);
   DBGL_CALL(glBindRenderbuffer,GL_RENDERBUFFER, _fbProps.RBO)
   DBGL_CALL(glRenderbufferStorage,GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, W, H) // use a single renderbuffer object for both a depth AND stencil buffer.
   DBGL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _fbProps.RBO) // now actually attach it
   // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
      return false;
   }
   return true;
}

void trend::Shaders::windowVAO()
{
   float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
       // positions   // texCoords
       -1.0f,  1.0f,  0.0f, 1.0f,
       -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

       -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
   };

   // screen quad VAO
   DBGL_CALL(glGenVertexArrays, 1, &_fbProps.quadVAO)
   DBGL_CALL(glGenBuffers, 1, &_fbProps.quadVBO)
   DBGL_CALL(glBindVertexArray,_fbProps.quadVAO)
   DBGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, _fbProps.quadVBO)
   DBGL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW)
   DBGL_CALL(glEnableVertexAttribArray, 0)
   DBGL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0)
   DBGL_CALL(glEnableVertexAttribArray, 1)
   DBGL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)))
}

void trend::Shaders::drawFrameBuffer()
{
   windowVAO();
   DBGL_CALL(glBindFramebuffer,GL_FRAMEBUFFER, 0)
   DBGL_CALL(glDisable,GL_DEPTH_TEST) // disable depth test so screen-space quad isn't discarded due to depth test.
   // clear all relevant buffers
   DBGL_CALL(glClearColor, .5f, .5f, .5f, .5f) // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
   DBGL_CALL(glClear, GL_COLOR_BUFFER_BIT)

   TRENDC->setGlslProg(glslp_FB);
   DBGL_CALL(glBindVertexArray, _fbProps.quadVAO)
   DBGL_CALL(glBindTexture, GL_TEXTURE_2D, _fbProps.texture)   // use the color attachment texture as the texture of the quad plane
   DBGL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6)
}


void trend::Shaders::clearFrameBuffer() {
   DBGL_CALL(glDeleteVertexArrays,1, &_fbProps.quadVAO)
   DBGL_CALL(glDeleteBuffers, 1, &_fbProps.quadVBO)
   DBGL_CALL(glDeleteTextures, 1, &_fbProps.texture)
   DBGL_CALL(glDeleteRenderbuffers, 1, &_fbProps.RBO)
   DBGL_CALL(glDeleteFramebuffers, 1, &_fbProps.FBO)
   _fbProps = {0,0,0,0,0};
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
   _dRenderer       (              NULL),
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
            default: tell_log(console::MT_ERROR, "Graphic rendering is not available! (see the log above). Nothing will be drawn."); break;//assert(false); break;
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
         wxLogDebug("Falling back to VBO rendering because of the errors above");
         tell_log(console::MT_WARNING, "Falling back to VBO rendering because of the errors above");
         _renderType = trend::rtTBD;//trend::rtTenderer;
      }
   }
}

void trend::TrendCenter::drawFrameBuffer()
{
   switch (_renderType) {
      case trend::rtToshader:
         assert(_cShaders);
         _cShaders->drawFrameBuffer();
         break;
      default: assert(false); // TODO handled for Toshader only at this stage
         break;
   }
   
}

trend::TrendBase* trend::TrendCenter::makeCRenderer(int W, int H)
{
   if (NULL != _cRenderer)
   {
      delete (_cRenderer);
      _cRenderer = NULL;
   }
   // Don't block the drawing if the databases are
   // locked. This will block all redraw activities including UI
   // which have nothing to do with the DB. Drop a message in the log
   // and keep going!
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
            _cRenderer = DEBUG_NEW trend::Toshader( drawProp, PROPC->UU() );
            if (!_cShaders->setFrameBuffer(W, H))
            {
               delete _cRenderer;
               _cRenderer = NULL;
            }
            break;
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

trend::TrendBase* trend::TrendCenter::getCRenderer()
{
   if (NULL != _cRenderer)
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->tryLockDrawProp(drawProp))
      {
         _cRenderer->setDrawProp(drawProp);
         return _cRenderer;
      }
      else
      {
//         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
         return NULL;
      }
   }
   return NULL;
}

trend::TrendBase* trend::TrendCenter::makeHRenderer()
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

trend::TrendBase* trend::TrendCenter::makeMRenderer(console::ACTIVE_OP& curOp)
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

trend::TrendBase* trend::TrendCenter::makeZRenderer()
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

trend::TrendBase* trend::TrendCenter::makeDRenderer()
{
   assert(NULL == _dRenderer);
   layprop::DrawProperties* drawProp;
   if (PROPC->tryLockDrawProp(drawProp, layprop::prsDRC))
   {
      switch (_renderType)
      {
      case trend::rtTocom: assert(false);          break;// shouldn't end-up here ever
      case trend::rtTolder:
         _dRenderer = DEBUG_NEW trend::Tolder(drawProp, PROPC->UU()); break;
      case trend::rtTenderer:
         _dRenderer = DEBUG_NEW trend::Tenderer(drawProp, PROPC->UU()); break;
      case trend::rtToshader:
         _dRenderer = DEBUG_NEW trend::Toshader(drawProp, PROPC->UU()); break;
      default: assert(false); break;
      }
   }
   return _dRenderer;
}

trend::TrendBase* trend::TrendCenter::getDRenderer()
{
   if (NULL != _dRenderer)
   {
      layprop::DrawProperties* drawProp;
      if (PROPC->tryLockDrawProp(drawProp, layprop::prsDRC))
      {
         _dRenderer->setDrawProp(drawProp);
         return _dRenderer;
      }
      else
      {
//         tell_log(console::MT_INFO,std::string("Property DB busy. Viewport redraw skipped"));
         return NULL;
      }
   }
   return NULL;
}

void trend::TrendCenter::releaseCRenderer()
{
   assert(NULL != _cRenderer);
   PROPC->unlockDrawProp(_cRenderer->drawprop(), false);
   if (_cRenderer->grcDataEmpty())
   {
      delete (_cRenderer);
      _cRenderer = NULL;
   }
}

void trend::TrendCenter::releaseDRenderer(bool destroy)
{
   assert(NULL != _dRenderer);
   PROPC->unlockDrawProp(_dRenderer->drawprop(), false);
   if (destroy || _dRenderer->grcDataEmpty())
   {
      delete (_cRenderer);
      _cRenderer = NULL;
   }
}

void trend::TrendCenter::destroyHRenderer()
{
   assert(NULL != _hRenderer);
   PROPC->unlockDrawProp(_hRenderer->drawprop(), false);
   delete (_hRenderer);
   _hRenderer = NULL;
}

void trend::TrendCenter::destroyMRenderer()
{
   assert(NULL != _mRenderer);
   PROPC->unlockDrawProp(_mRenderer->drawprop(), false);
   delete (_mRenderer);
   _mRenderer = NULL;
}

void trend::TrendCenter::destroyZRenderer()
{
   assert(NULL != _zRenderer);
   PROPC->unlockDrawProp(_zRenderer->drawprop(), false);
   delete (_zRenderer);
   _zRenderer = NULL;
}

void trend::TrendCenter::loadLayoutFont(std::string fontfile)
{
   // Parse the font library
   TolderGlfFont* curFont = NULL;
   switch (_renderType)
   {
      case trend::rtTocom    : assert(false);          break;
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
   if (NULL != _oglFont[_activeFontName])
      _oglFont[_activeFontName]->getStringBounds(text, overlap);
   else // in case the fonts are not initialized for whatever reason (shader issue for example)
      *overlap = {{0,0},{0,0}};

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
   DBGL_CALL(glBindBuffer,GL_ARRAY_BUFFER, 0)
   DBGL_CALL(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER, 0)
}

void trend::TrendCenter::setUniVarf(const glsl_Uniforms varName, GLfloat varValue) const
{
   assert(_cShaders);
   return _cShaders->setUniVarf(varName, varValue);
}

void trend::TrendCenter::setUniMtrx4fv(const glsl_Uniforms varName, GLfloat* varValue) const
{
   assert(_cShaders);
   return _cShaders->setUniMtrx4fv(varName, varValue);
}

void trend::TrendCenter::setUniColor(/*const glsl_Uniforms varName,*/ GLfloat* varValue) const
{
   assert(_cShaders);
   return _cShaders->setUniColor(varValue);
}

void trend::TrendCenter::setUniStipple(/*const glsl_Uniforms varName,*/ GLuint* varValue) const
{
   assert(_cShaders);
   return _cShaders->setUniStipple(varValue);
}

void trend::TrendCenter::setUniVarui(const glsl_Uniforms varName, GLuint varValue) const
{
   assert(_cShaders);
   return _cShaders->setUniVarui(varName, varValue);
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

