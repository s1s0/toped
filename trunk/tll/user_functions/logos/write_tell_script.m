#!/usr/bin/octave

# this octave script writes polygons to a Tell-function.
# all polygons extracted with the help of "potrace" contains information of 
# beeing a holes in an other polygon or beeing an standalone polygon
# A algorthmic search is done to determine and connect inner with outer
# polygons to make it drawable in Toped
#
# Copyright (C) 2010 by Armin Taschwer

# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA




if (nargin < 2)
  printf ("Error: Usage: ./write_tell_script  <data-file> <out-file.tell>\n");
  exit (1);
endif

data_f = nth(argv, 1);
tell_f = nth(argv, 2);

source ( data_f );

source ("config.m");

source ("write_tell_script_helper.m");


## the data are now organized in a structure: A(n).poly = [..]
## they have to be copied into another structure

p = struct ("polyX", [], "polyY", [], "inner", 0);


for i = 1:max(size(A))
    l = columns ( A(i).poly );
    if ( mod(l,2) == 1)
	len = l-1;
	p(i).inner = 1;
    else
	len = l-2;
	p(i).inner = 0;
    endif
    pp = A(i).poly(1:len);
    
    od = 1:2:len;
    ev = 2:2:len;
    [p(i).polyX, p(i).polyY] = simplify_polygon (pp(od)', pp(ev)');

endfor

## vertical vectors demanded!
newp = normalize_polygon_size (p);

nnewp = merge_all_polygons (newp);

r = create_tell_code (nnewp, tell_f );
if (r == -1)
  printf ("ERROR: can't create TELL-file %s\n", tell_f);
  exit(1);
endif
