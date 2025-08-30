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

uniform vec4      in_Color          ;
uniform sampler2D layTexture        ;
uniform bool      textureON = false ;

// Interpolated values from the vertex shaders
in      vec2      UV                ;
out     vec4      color             ;

vec4 hash4( vec2 p )
{
   return fract(sin(vec4( 1.0+dot(p,vec2(37.0,17.0)),
                          2.0+dot(p,vec2(11.0,47.0)),
                          3.0+dot(p,vec2(41.0,29.0)),
                          4.0+dot(p,vec2(23.0,31.0))))*103.0);
}

vec4 textureNoTile( sampler2D samp, in vec2 uv, float v )
// Taken from https://www.shadertoy.com/view/4tsGzf
// The MIT License Copyright © 2015 Inigo Quilez
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions: The above copyright notice and
// this permission notice shall be included in all copies or substantial
// portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT
// WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org
{
   vec2 p = floor( uv );
   vec2 f = fract( uv );

   // derivatives (for correct mipmapping)
   vec2 ddx = dFdx( uv );
   vec2 ddy = dFdy( uv );

   vec3  va = vec3(0.0);
   float w1 = 0.0;
   float w2 = 0.0;
   for( int j=-1; j<=1; j++ )
      for( int i=-1; i<=1; i++ )
      {
         vec2  g = vec2( float(i),float(j) );
         vec4  o = hash4( p + g );
         vec2  r = g - f + o.xy;
         float d = dot(r,r);
         float w = exp(-5.0*d );
         vec3  c = textureGrad( samp, uv + v*o.zw, ddx, ddy ).xyz;
         va += w*c;
         w1 += w;
         w2 += w*w;
      }

    // normal averaging --> lowers contrasts
    //return va/w1;

   // contrast preserving average
   float mean = textureGrad( samp, uv, ddx*16.0, ddy*16.0 ).x;
   vec3 res = mean + (va-w1*mean)/sqrt(w2);
   return vec4(mix( va/w1, res, v ),0.8);
}


void main()
{
   if (textureON)
   {
//      color = texture( layTexture, UV );// * in_Color;
//      float rand = random(UV);
      color = textureNoTile( layTexture, UV, 0.7 ) * in_Color;
   }
   else
      color = in_Color;
}
