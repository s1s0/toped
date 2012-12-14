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
//           $URL: https://toped.googlecode.com/svn/trunk/shaders/fragment.glsl $
//        Created: Sat Dec  8 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: GLSL Fragment shader
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


void main()
{
   gl_Position = gl_in[0].gl_Position;
   EmitVertex();
   gl_Position = gl_in[1].gl_Position;
   EmitVertex();
   EndPrimitive();
}