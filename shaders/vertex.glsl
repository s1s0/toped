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
//   This file is a part of Toped project (C) 2001-2009 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sat Dec  8 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL Vertex shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#version 330

layout (location = 0) in      vec2  in_Vertex;
// Implicit
//out gl_PerVertex {
//    vec4 gl_Position;
//    float gl_PointSize;
//    float gl_ClipDistance[];
//};
//<= ERROR: Input of fragment shader 'patternCoord' not written by vertex shader
//ERROR: Input of fragment shader 'markCoord' not written by vertex shader

uniform mat4 in_CTM;
uniform float in_Z = 0;
noperspective out vec2 markCoord;
noperspective out float patternCoord;
void main(void)
{
   gl_Position = in_CTM * vec4(in_Vertex, in_Z, 1.0);
   markCoord = vec2(0,0);
   patternCoord = 0;
}
