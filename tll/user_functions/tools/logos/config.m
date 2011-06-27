## not a function file
1;

## EXAMPLE - configuration file for usage together with 
## script file "write_tell_scrip.m"
##
## Copyright (C) 2010 by Armin Taschwer 

## structured array containing the layer names which should be used in TELL

global LAYER = struct ( "name", "");

#LAYER(1).name = "MET1";
#LAYER(2).name = "MET2";
#LAYER(3).name = "MET3";
#LAYER(4).name = "MET4";

LAYER(1).name = "MET4";

## used to scale the logo, maximal dimension is normalized
## any number will scale your logo to xx um (depending on the units)

global SIZE = 100;
