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
//           $URL$
//        Created: Sun Jan  6 2013
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL Geometry shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#version 330

layout(points) in;

uniform vec2  in_ScreenSize;
uniform float ShSize = 15;
layout(triangle_strip, max_vertices=4) out;

noperspective out vec2 markCoord;
void main()
{
   float sizeX = ShSize / in_ScreenSize.x ;
   float sizeY = ShSize / in_ScreenSize.y ;

   gl_Position = gl_in[0].gl_Position + vec4(-sizeX, -sizeY, 0.0, 0.0);
   markCoord = vec2(0,0);
   EmitVertex();
   
   gl_Position = gl_in[0].gl_Position + vec4(sizeX, -sizeY, 0.0, 0.0);
   markCoord = vec2(ShSize,0);
   EmitVertex();

   gl_Position = gl_in[0].gl_Position + vec4(sizeX, sizeY, 0.0, 0.0);
   markCoord = uvec2(ShSize,ShSize);
   EmitVertex();

   gl_Position = gl_in[0].gl_Position + vec4(-sizeX, sizeY, 0.0, 0.0);
   markCoord = vec2(0,ShSize);
   EmitVertex();

   EndPrimitive();
}