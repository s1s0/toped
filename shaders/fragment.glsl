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
//    Description: GLSL Fragment shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#version 330

uniform vec3  in_Color;
uniform float in_Alpha;
uniform uint  in_Stipple[33];
uniform uint  in_LStipple = uint(0x00000ff0);
uniform bool  in_StippleEn = false;
uniform bool  in_LStippleEn = false;

//in  vec4 gl_FragCoord;
noperspective in float patternCoord;
out vec4 out_Color;

void main(void)
{
   bool dropThePixel = false;
   if (in_StippleEn)
   {
      uvec2 ufCoord = uvec2(gl_FragCoord.x, gl_FragCoord.y);
      uint index = 31u - (ufCoord.y % 32u);
      uint mask  = uint(0x80000000) >> (ufCoord.x % 32u);
      dropThePixel = !bool(in_Stipple[index+uint(1)] & mask);
   }
   else if (in_LStippleEn)
   {
      uint pcoord = uint(patternCoord);
      uint mask  = uint(0x00008000) >> (pcoord % 16u);
      dropThePixel = !bool(in_LStipple & mask);
   }
   if (dropThePixel)
      discard;
   else
      out_Color = vec4(in_Color,in_Alpha);
}
