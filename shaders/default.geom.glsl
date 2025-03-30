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
//   This file is a part of Toped project (C) 2001-2013 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL: https://toped.googlecode.com/svn/trunk/shaders/fragment.glsl $
//        Created: Sat Dec  8 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL Geometry shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 2327 $
//          $Date: 2012-12-08 18:39:40 +0000 (Sat, 08 Dec 2012) $
//        $Author: krustev.svilen $
//===========================================================================

#version 330

layout(lines) in;
layout(line_strip, max_vertices=2) out;

uniform vec2 in_ScreenSize;
uniform uint in_PatScale = 1u;

noperspective out float patternCoord;
noperspective out vec2 markCoord;

//Implicit
//in gl_PerVertex
//{
//  vec4 gl_Position;
//  float gl_PointSize;
//  float gl_ClipDistance[];
//} gl_in[gl_MaxPatchVertices];

void main()
{
   vec2 p0Pos = in_ScreenSize.xy * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
   vec2 p1Pos = in_ScreenSize.xy * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
   markCoord = vec2(0,0);

   gl_Position = gl_in[0].gl_Position;
   patternCoord = 0.0;
   EmitVertex();
   
   gl_Position = gl_in[1].gl_Position;
   patternCoord = 0.5 * length(p1Pos - p0Pos) / float(in_PatScale);
   EmitVertex();

   EndPrimitive();
}
