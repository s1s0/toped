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
//        Created: Sun Jul 27 2025
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL 3D Fragment shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#version 330 core

uniform vec4  in_Color                        ;
uniform sampler2D layTexture;
uniform bool textureON = false;

// Interpolated values from the vertex shaders
in vec2 UV;
out vec4 color;

void main()
{
	// Output color = color of the texture at the specified UV
   //color = /*texture( myTextureSampler, UV ).rgb * */ in_Color/*.rgb*/;
   vec4 colorOnly = in_Color;
	vec4 colorANDtexture = texture( layTexture, UV ) * in_Color ;
   if (textureON)
      color = colorANDtexture;
   else
      color = colorOnly;
      
}
