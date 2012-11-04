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


trend::FontLibrary*              fontLib = NULL;
trend::TrendCenter*              TRENDC  = NULL;
extern layprop::PropertyCenter*  PROPC;
extern trend::GlslUniVarLoc      glslUniVarLoc;

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

void trend::TGlfRSymbol::draw(bool fill)
{
   glMultiDrawArrays(GL_LINE_LOOP, _firstvx, _csize, _alcntrs);
   if (!fill) return;
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
trend::TGlfFont::TGlfFont(std::string filename, std::string& fontname) :
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

void trend::TGlfFont::getStringBounds(const std::string& text, DBbox* overlap)
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

void trend::TGlfFont::drawString(const std::string& text, bool fill)
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

trend::TGlfFont::~TGlfFont()
{
   for (TFontMap::const_iterator CS = _tsymbols.begin(); CS != _tsymbols.end(); CS++)
      delete (CS->second);
}

//=============================================================================
//
//
//
trend::TGlfVboFont::TGlfVboFont(std::string filename, std::string& fontname) :
   TGlfFont   ( filename, fontname )
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

void trend::TGlfVboFont::collect(const word all_vertexes, const word all_indexes)
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

bool trend::TGlfVboFont::bindBuffers()
{
   if ((0 ==_pbuffer) || (0 == _ibuffer)) return false;
   glBindBuffer(GL_ARRAY_BUFFER, _pbuffer);
//   GLint bufferSize;
//   glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
//   bufferSize++;

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibuffer);
//   glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
//   bufferSize++;

   return true;
}

void trend::TGlfVboFont::getStringBounds(const std::string& text, DBbox* overlap)
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

void trend::TGlfVboFont::drawString(const std::string& text, bool fill)
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
         CSI->second->draw(fill);
         right_of = CSI->second->maxX();
      }
   }
   if (fill)
      glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
}

trend::TGlfVboFont::~TGlfVboFont()
{
   for (FontMap::const_iterator CS = _symbols.begin(); CS != _symbols.end(); CS++)
      delete (CS->second);
   GLuint ogl_buffers[2] = {_pbuffer, _ibuffer};
   glDeleteBuffers(2, ogl_buffers);
}

//=============================================================================
//
// FontLibrary
//
trend::FontLibrary::FontLibrary(RenderType renderType) :
   _activeFontName  (          ),
   _renderType      (renderType)
{
}

bool trend::FontLibrary::loadLayoutFont(std::string fontfile)
{
   // Parse the font library
   TGlfFont* curFont = NULL;
   switch (_renderType)
   {
      case trend::tocom    : assert(false);          break;// TODO?
      case trend::tolder   :
         curFont = DEBUG_NEW trend::TGlfFont(fontfile, _activeFontName);
         break;
      case trend::tenderer :
         curFont = DEBUG_NEW trend::TGlfVboFont(fontfile, _activeFontName);
         break;
      case trend::toshader :
//         curFont = DEBUG_NEW trend::TGlfShrFont(fontfile, _activeFontName); //TODO
         assert(false);
         break;
      default: assert(false); break;
   }

   if (!curFont->status())
   {
      _oglFont[_activeFontName] = curFont;
      return true;
   }
   return false;
}

bool trend::FontLibrary::selectFont(std::string fname)
{
   if (_oglFont.end() != _oglFont.find(fname))
   {
      _activeFontName = fname;
      return true;
   }
   return false;
}

void trend::FontLibrary::getStringBounds(const std::string& text, DBbox* overlap)
{
   assert(NULL != _oglFont[_activeFontName]); // make sure that fonts are initialised
   _oglFont[_activeFontName]->getStringBounds(text, overlap);
}

void trend::FontLibrary::drawString(const std::string& text, bool fill)
{
   _oglFont[_activeFontName]->drawString(text, fill);
}

void trend::FontLibrary::drawWiredString(const std::string& text)
{
   bindFont();
   _oglFont[_activeFontName]->drawString(text, false);
   unbindFont();
}

void trend::FontLibrary::drawSolidString(const std::string& text)
{
   bindFont();
   _oglFont[_activeFontName]->drawString(text, true);
   unbindFont();
}

void trend::FontLibrary::bindFont()
{
   if (NULL != _oglFont[_activeFontName])
      _oglFont[_activeFontName]->bindBuffers();
}

void  trend::FontLibrary::unbindFont()
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

word trend::FontLibrary::numFonts()
{
   return _oglFont.size();
}

trend::FontLibrary::~FontLibrary()
{
   for (OglFontCollectionMap::const_iterator CF = _oglFont.begin(); CF != _oglFont.end(); CF++)
      delete (CF->second);
}

//=============================================================================
trend::Shaders::Shaders() :
   _fnShdrVertex     ( "vertex.glsl"      ),
   _fnShdrFragment   ( "fragment.glsl"    ),
   _idShdrVertex     ( -1                 ),
   _idShdrFragment   ( -1                 ),
   _status           ( true               )
{
   // initialize all uniform variables names
   _glslUniVarNames[glslu_in_CTM]       = "in_CTM";
   _glslUniVarNames[glslu_in_Z]         = "in_Z";
   _glslUniVarNames[glslu_in_Color]     = "in_Color";
   _glslUniVarNames[glslu_in_Alpha]     = "in_Alpha";
   _glslUniVarNames[glslu_in_Stipple]   = "in_Stipple";
   _glslUniVarNames[glslu_in_StippleEn] = "in_StippleEn";
}

void trend::Shaders::loadShadersCode(const std::string& codeDirectory)
{
   _fnShdrVertex   = codeDirectory + _fnShdrVertex;
   _fnShdrFragment = codeDirectory + _fnShdrFragment;
   if(  (_status &= compileShader(_fnShdrVertex  , _idShdrVertex  , GL_VERTEX_SHADER  ))
      &&(_status &= compileShader(_fnShdrFragment, _idShdrFragment, GL_FRAGMENT_SHADER))
     )
   {
      GLuint idProgram = glCreateProgram();
      if (0 == idProgram)
      {
         _status = false;
         return;
      }

      // TODO bind attribute locations

      // attach the shaders
      glAttachShader(idProgram, _idShdrVertex  );
      glAttachShader(idProgram, _idShdrFragment);
      // link
      glLinkProgram(idProgram);
      GLint programOK;
      glGetProgramiv(idProgram, GL_LINK_STATUS, &programOK);
      if (programOK)
      {
         std::stringstream info;
         info << "GLSL program linked";
         tell_log(console::MT_INFO, info.str());
         _status = bindUniforms(idProgram);
      }
      else
      {
         std::stringstream info;
         info << "GLSL linking failed. See the log below:";
         tell_log(console::MT_ERROR, info.str());
         getProgramsLog(idProgram);
         _status = false;
      }
      if (_status)
         glUseProgram(idProgram);
   }
}

bool trend::Shaders::bindUniforms(GLuint idProgram)
{
   for (GlslUniVarNames::const_iterator CV = _glslUniVarNames.begin(); CV != _glslUniVarNames.end(); CV++)
   {
      GLuint location = glGetUniformLocation(idProgram, CV->second.c_str());
      if( location >= 0 )
      {
         glslUniVarLoc[CV->first] = location;
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
      delete infoLog;
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
      delete infoLog;
   }
}

trend::Shaders::~Shaders()
{
}

//=============================================================================
trend::TrendCenter::TrendCenter(bool gui, bool forceBasic, bool sprtVbo, bool sprtShaders) :
   _cRenderer     (              NULL),
   _hRenderer     (              NULL),
   _cShaders      (              NULL)
{
   if      (!gui)             _renderType = trend::tocom;
   else if ( forceBasic )     _renderType = trend::tolder;
   else if ( sprtShaders)     _renderType = trend::toshader;
   else if ( sprtVbo    )     _renderType = trend::tenderer;
   else                       _renderType = trend::tolder;
   if (trend::toshader== _renderType)
      _cShaders = DEBUG_NEW trend::Shaders();
   fontLib = DEBUG_NEW trend::FontLibrary(_renderType);

//   switch (_renderType)
//   {
//      case trend::tocom    : assert(false);          break;// TODO?
//      case trend::tolder   :
//         fontLib = DEBUG_NEW trend::RpFontLib();     break;
//      case trend::tenderer :
//         fontLib = DEBUG_NEW trend::FontLibrary();     break;
//      case trend::toshader :
//         fontLib = DEBUG_NEW trend::FontLibrary();     break;
//      default: assert(false); break;
//   }
}

void trend::TrendCenter::initShaders(const std::string& codeDirectory)
{
   if (trend::toshader == _renderType)
   {
      assert(_cShaders);
      _cShaders->loadShadersCode(codeDirectory);
      if (!_cShaders->status())
      {
         tell_log(console::MT_WARNING, "Falling back to VBO rendering because of the errors above");
         _renderType = trend::tenderer;
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
         case trend::tocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::tolder   :
            _cRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() );break;
         case trend::tenderer :
            _cRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::toshader : 
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
         case trend::tocom    : assert(false);          break;// shouldn't end-up here ever
         case trend::tolder   :
            _hRenderer = DEBUG_NEW trend::Tolder( drawProp, PROPC->UU() ); break;
         case trend::tenderer :
            _hRenderer = DEBUG_NEW trend::Tenderer( drawProp, PROPC->UU() ); break;
         case trend::toshader : assert(false);          break;// TODO
         default: assert(false); break;
      }
   }
   return _hRenderer;
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

void trend::TrendCenter::drawFOnly()
{
//   if (NULL != _cRenderer) _cRenderer->grcDraw(); // TODO
}

void trend::TrendCenter::drawGrid()
{
   // render the grid
   for (byte gridNo = 0; gridNo < 3; gridNo++)
   {
      const layprop::LayoutGrid* cgrid = PROPC->grid(gridNo);
      if ((NULL !=  cgrid) && cgrid->visual())
         _cRenderer->grid(cgrid->step(), cgrid->color());
   }
}

void trend::TrendCenter::drawZeroCross()
{
   if (PROPC->zeroCross())
      _cRenderer->zeroCross();
}

trend::TrendCenter::~TrendCenter()
{
   delete fontLib;
   if (NULL != _cRenderer) delete _cRenderer;
}
