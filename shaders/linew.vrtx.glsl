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
//        Created: Sat Mar 29 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL Vertex shader generating lines with arbitrary width
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
// idea taken from https://stackoverflow.com/questions/3484260/opengl-line-width/59688394#59688394
// but optimized
#version 330

#define vrtxNum 4

uniform mat4  in_CTM;
uniform float in_Z = 0;
uniform vec2  in_ScreenSize;
uniform float in_LineWidth;
//in vec3 vCol[2];
in vec2 in_Vertex[vrtxNum];
//out vec3 color;
noperspective out vec2 markCoord;
noperspective out float patternCoord;

void main()
{
   gl_PointSize = 10;

   vec4 va[vrtxNum];
   vec4 pos;
   vec2 v_pred;

   for (int i=0; i<vrtxNum; ++i)
   {
      va[i] = in_CTM * vec4(in_Vertex[i],in_Z,1.0);
      va[i].xy = (va[i].xy + 1.0) * 0.5 * in_ScreenSize;
   }

   vec2 v_line  = normalize(va[2].xy - va[1].xy);
   vec2 nv_line = vec2(-v_line.y, v_line.x);
   if (gl_VertexID < 2)
   {// means for the first segment only - process first two vertexes
      v_pred = (va[1].xy == va[0].xy) ? v_line : normalize(va[1].xy - va[0].xy);
      pos = va[1];
   }
   else
   {// for all consequitive segments - process only second pair of vertexes
      v_pred = (va[3].xy == va[2].xy) ? v_line : normalize(va[3].xy - va[2].xy);
      pos = va[2];
      
   }
   
   vec2 v_miter = normalize(nv_line + vec2(-v_pred.y, v_pred.x));
   pos.xy += v_miter * in_LineWidth * ((gl_VertexID%2) == 0 ? -0.5 : 0.5) / dot(v_miter, nv_line);
   pos.xy = pos.xy / in_ScreenSize * 2.0 - 1.0;

   markCoord = vec2(0,0);
   patternCoord = 0;
   gl_Position = pos;
//   color = vCol[1];
}

//WARNING: Output of vertex shader 'color' not read by fragment shader
//ERROR: Input of fragment shader 'patternCoord' not written by vertex shader
//ERROR: Input of fragment shader 'markCoord' not written by vertex shader
