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
//    Description: GLSL 3D Vertex shader
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================
#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexStream;
//layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 CTM;
uniform mat3 TEXMAT;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * CTM * vec4(vertexStream,1);
	vec3 texPos = TEXMAT * vertexStream;
	// UV of the vertex. No special space for this one.
	UV = texPos.xy;
}

