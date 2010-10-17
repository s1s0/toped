
## not a function file
1;

# this **octave script** writes polygons to a Tell-function.
# all polygons extracted with the help of "potrace" contains information of 
# beeing a holes in an other polygon or beeing an standalone polygon
# A algorthmic search is done to determine and connect inner with outer
# polygons to make it drawable in Toped
#
# here some helper functions

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


## cross product of 2D vectors
## @param a input vector a = [ x ; y ]
## @param b input vector b = [ x1; y2]
## @return cross product between a and b
function c = cross_2d (a, b)
  c = a(1)*b(2) - b(1)*a(2);
endfunction

## absolut value of an 2D vector
## @param a input vector a = [ x ; y ]
## @return length of vector a (in complex domain this would be the abs-value)
function a = abs_2d (a)
  a = sqrt(a(1)*a(1) + a(2)*a(2));
endfunction

## distance between two points
## @param a first vector a = [ x1 ; y1 ]
## @param b sec. vector b = [ x2 ; y2 ]
function len = dist_2d ( a, b)
  len = abs_2d (b .- a);
endfunction

## determines if a point is on the edge of two neighbour-points
## @param p1 point to check
## @param p0 first neighbour
## @param p2 second neighbour
## @return 1 if point is on the edge 
function onedge = point_is_on_edge ( p1, p0, p2)
  EPS = 1e-20; ## try to reduce errors to calculation noise
  a = p2' .- p0';
  b = p1' .- p0';
  # find the minimal distance of p1 to the edge
  # dist = (a x b) / a

  dist_a = abs_2d(a);

  if (dist_a == 0) ## not a line
    dist = abs_2d(b);
    if ( dist > 0 )
      onedge = 0;
    else
      onedge = 1;
    endif
  else
    dist = cross_2d(a,b) / dist_a;

    onedge = 0;
  endif

  if (abs(dist) < EPS & onedge == 0)
    ## distance is now zero! is point between p0 and p2 ?
    A = p0; ## create a rectangle: left-bottom = A
    B = p2; ## right-top = B
    if (a(1) < 0) A(1) = p2(1); B(1) = p0(1); endif
    if (a(2) < 0) A(2) = p2(2); B(2) = p0(2); endif

    if (p1(1) >= A(1) & p1(1) <= B(1) &
	p1(2) >= A(2) & p1(2) <= B(2) )
      onedge = 1;
    endif
  endif
endfunction

## tries to reduce the number of point of an polygon
## if a point between two neigbour-points lies exactly on their edge
## this point can be deleted
## @param x .. x points of polygon
## @param y .. y points of polygon
## @return px,py .. two vectors containing the reduced polygon
function [px,py] = simplify_polygon (x, y)
  l = length (x);
  if (l >= 3)
    px(1) = x(1);
    py(1) = y(1);
    count = 2;

    for i = 2:l
      if (i < l)
	if (point_is_on_edge( [x(i),y(i)], [x(i-1),y(i-1)], [x(i+1), y(i+1)] ) == 0 )
	  px(count) = x(i) ;
	  py(count) = y(i) ;
	  count = count + 1;
	endif
      endif
    endfor
    px(count) = x(l);
    py(count) = y(l);
  endif
endfunction


## looks for the maximum
## @param p contains list of polygon in a struct-array: p(n).{polyX polyY inner} 
## @return pst is a struct-array: pst(n).{polyX polyY inner len}
function pst = normalize_polygon_size (p)
    l = length (p);
    minxA = inf;
    minyA = inf;
    maxxA = -inf;
    maxyA = -inf;
    for i = 1:l
	maxx = max(p(i).polyX);
	maxy = max(p(i).polyY);
	minx = min(p(i).polyX);
	miny = min(p(i).polyY);
	minxA = min (minx, minxA);
	minyA = min (miny, minyA);
	maxxA = max (maxx, maxxA);
	maxyA = max (maxy, maxyA);
    endfor
    
    scaleX = maxxA - minxA;
    scaleY = maxyA - minyA;
    scale = max(scaleX,scaleY);

    midX = (maxxA + minxA) / 2;
    midY = (maxyA + minyA) / 2;
    
    for i = 1:l
	pst(i).polyX = (p(i).polyX .- midX) ./ scale;
        ## the minus sign flips the polygon around the medium x-axis
	pst(i).polyY = -(p(i).polyY .- midY) ./ scale;
	pst(i).inner = p(i).inner;
	pst(i).len = length(p(i).polyX);
    endfor	
endfunction


## merge two polygons
## precondition is that the polygon A is within B
## @param ax x members of polyon A which should be the inner polygon
## @param ay y members of polyon A which should be the inner polygon
## @param bx x members of polygon B
## @param by y members of polygon B
## @return [ x , y ] the new polygon
function [x,y] = merge_polygon (ax, ay, bx, by)

  if (rows(ax) < columns(ax))
    error("ERROR: merge_polygon: vertical vectors needed");
  endif

  lip = max(size(ax));
  lep = max(size(bx));
  mindist = [inf,1,1]; ## minimal distance, idx ext. polyg., idx inner polyg

  l = inpolygon (ax,ay, bx, by); ##returns boolean vector (each position 1 or 0)

  # debug
  # ax
  # ay
  # bx 
  # by
  # l

  if ( sum(l) == lip & sum(l) >= 3 & max(size((bx))) >= 3)  ## polygon A is inside B)

    ## search for the minimum distance between polygon A and B
    ## for now very time-consuming
    for p = 1:lep
      for i = 1:lip
	d = dist_2d( [ ax(i) ; ay(i) ], [ bx(p) ; by(p) ] );
	if ( d  < mindist(1) )
	  mindist = [d, p, i];
	endif 
      endfor
    endfor
    
    me = mindist(2);
    mi = mindist(3);

    ## A1..A3 can be empty vectors!
    A1 = [ ax(1:mi-1) , ay(1:mi-1)];
    A1l = mi - 1;
    A2 = [ ax(mi) , ay(mi) ];
    A3 = [ ax(mi+1:rows(ax)), ay(mi+1:rows(ay)) ];
    A3l = rows(ax) - mi;
    B1 = [ bx(1:me-1) , by(1:me-1)];
    B1l = me - 1;
    B2 = [ bx(me), by(me) ];
    B3 = [ bx(me+1:rows(bx)), by(me+1:rows(by)) ];
    B3l = rows(bx) - me;

    ## now merge everything
    final(1:B1l,:) = B1;
    final(B1l+1,:) = B2;
    c = B1l+1+1;
    final(c, :) = A2;
    final(c+1:c+A3l, :) = A3;
    c = c+A3l;
    final(c+1:c+A1l, :) = A1;
    c = c+A1l;
    final(c+1, :) = A2;
    final(c+2, :) = B2;
    final(c+3:c+2+B3l, :) = B3;
    
    #final
    
    x = final (:,1);
    y = final (:,2);
  else
    ## polygon hast to be at least 3 points
    ## thus the size of [x,y] indicates an error
    x = 0;
    y = 0;
  endif
  
endfunction

## check polygons and merge them if they are mergeable
## @param pin list of polygons in the form of struct (polyX, polyY, inner, len)
## @param pin list of merged polygons in the form of a struct (polyX, polyY)
function newp = merge_all_polygons (pin)

  len_p = length (pin);
  ci = 1; ## counter
  ce = 1; ## counter
  for i = 1:len_p
    if (pin(i).inner == 1)
      innerpoly(ci) = pin(i);
      ci = ci + 1;
    else
      extpoly(ce) = pin(i);
      ce = ce + 1;
    endif
  endfor

  len_pe = length (extpoly);
  len_pi = length (innerpoly);
  cn = 1; ## counter

  ## test each external polygon with an inner one
  for e = 1:len_pe
    merged = 0;
    for i = 1:len_pi
      [x,y] = merge_polygon (innerpoly(i).polyX', innerpoly(i).polyY',
			    extpoly(e).polyX', extpoly(e).polyY');
      if (length(x) >= 3)
	newp(cn).polyX = x;
	newp(cn).polyY = y;
	merged = 1;
	cn = cn + 1;
      endif

      if (i == len_pi & merged == 0)
	newp(cn).polyX = extpoly(e).polyX;
	newp(cn).polyY = extpoly(e).polyY;
	cn = cn + 1;
      endif

    endfor
  endfor

endfunction


## create polygons in toped 
## code is placed as a function into the workspace
## usage is simple: call "paint_logo"
## @param p polygons as a struct-array: p(n).{polyX polyY}
## @param file filename into which the function "paint_loop" should be 
## written
## @return if -1 .. something is wrong: primarly file access errors
function ret = create_tell_code (p, file)
  global SIZE;
  global LAYER;

  fid = fopen (file, "w");
  ret = 0;
  
  if (fid == -1)
    ret = -1;
    return;
  endif

  fprintf(fid, "/* this file is generated automatically by an octave-scripts */\n");
  fprintf(fid, "void paint_logo () {\n");
  
  
  l = length (p);
  ## without simplification of the polygons
  # for i = 1:l
  #   fprintf (fid, "  point list p%d = {", i);
  #   for j = 1:p(i).len
  #     # SIZE defined in configuration file
  #     fprintf (fid, "{ %f, %f }", 
  # 	       SIZE .* p(i).polyX(j), 
  # 	       SIZE .* p(i).polyY(j));
  #     if (j < p(i).len)
  # 	fprintf (fid, ",");
  #     endif
  #   endfor
  #   fprintf (fid, "};\n");
  # endfor

  ## this starts to write the polygon with some simplifications
  for i = 1:l
    fprintf (fid, "  point list p%d = {", i);

    sx = p(i).polyX;
    sy = p(i).polyY;
    sl = length(sx);

    for j = 1:sl
      # SIZE defined in configuration file
      fprintf (fid, "{ %f, %f }", 
  	       SIZE .* sx(j), 
  	       SIZE .* sy(j));
      if (j < sl)
  	fprintf (fid, ",");
      endif
    endfor
    fprintf (fid, "};\n");
  endfor

  ## uses layer informations from configuration file
  ll = length (LAYER);
  for lay = 1:ll
    fprintf (fid, "  usinglayer(\"%s\");\n", LAYER(lay).name);
    for i = 1:l
      fprintf (fid, "  addpoly ( round_list_to_grid (p%d) );\n", i);
    endfor
  endfor

  fprintf (fid, "}\n");
  fclose(fid);
  
endfunction
