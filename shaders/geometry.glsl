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

layout (lines) in;                              // now we can access 2 vertices
layout (triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)

//uniform vec2  u_viewportSize;
uniform float in_LWidth = 1;
uniform vec2 in_ScreenSize;
uniform uint in_PatScale = 1u;

noperspective out float patternCoord;
noperspective out vec2 markCoord;

void main()
{
//   patternCoord = 0.0;
   markCoord = vec2(0,0);
   vec2 p1Pos = in_ScreenSize.xy * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
   vec2 p2Pos = in_ScreenSize.xy * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
   vec4 p1 = gl_in[0].gl_Position;
   vec4 p2 = gl_in[1].gl_Position;

   vec2 dir    = normalize((p2.xy/p2.w - p1.xy/p1.w) * in_ScreenSize);
   vec2 offset = vec2(-dir.y, dir.x) * in_LWidth / in_ScreenSize;

   gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);
   patternCoord = 0.0;
   EmitVertex();
   gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);
   EmitVertex();
   gl_Position = p2 + vec4(offset.xy * p2.w, 0.0, 0.0);
   patternCoord = 0.5 * length(p2Pos - p1Pos) / float(in_PatScale);
   EmitVertex();
   gl_Position = p2 - vec4(offset.xy * p2.w, 0.0, 0.0);
   EmitVertex();
   
   EndPrimitive();
}
