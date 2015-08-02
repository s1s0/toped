# Introduction #

Most actual reference for functions and options in the development branch of Toped

# Details #

## Command line options ##

Usage: `toped {options}* [tll-file]`

**Command line options:**

```
  -ogl_thread: 
  -ogl_safe  : 
  -nolog     : logging will be suppressed //rev1917
  -nogui     : GUI will not be started. Useful for TLL parsing
  -I<path>   : includes additional search paths for TLL files 
               (multiple entries allowed)
  -D<macro>  : equivalent to #define <macro> (multiple entries allowed)
  -help      : This help message
```

## Environment variables ##

Also described in in http://www.toped.org.uk/trm_intro.html

```
  TPD_LOCAL .. user specific path, mainly used to store log files
  TPD_GLOBAL .. global path to a proper toped installation to find fonts, etc. 
  TLL_INCLUDE_PATH .. list of several paths (seperated by :) for describing 
                      the location of UDFs (equivalent to -I in the command line)
```

if not set the default path will always be the current path.

## Preprocessing commands ##

**Pragmas**

bug fix for "fullreset" in [r2115](https://code.google.com/p/toped/source/detail?r=2115) (fixes [issue 136](https://code.google.com/p/toped/issues/detail?id=136))

```
  #pragma once   //r1911: calls the *.tll file only once in actual toped session
  #pragma prereset //r1911: 
  #pragma fullreset //r1920: deletes any function or variable definitions. TELL stack is "clean" 
```

**Definitions**

```
  #define <var> <value>
  #define <macro> <code>
```

**Statements**

```
  #ifdef | #ifndef 
  [#else]
  #endif
```

## Tell-language ##

### Data-Types ###
#### Lists ####

  * Speed issues: [issue 110](https://code.google.com/p/toped/issues/detail?id=110), [r1907](https://code.google.com/p/toped/source/detail?r=1907)

preallocation of lists allows a significant improvement in terms of
execution speed when an application uses dynamic growing/shrinking lists

#### Points ####

  * operations on points like "a + {1,2}" are not that straightforward
    * one have to use "a + (point){1,2}"  (point of introduction unknown)
    * but attention, concept of explicit typecasting is not introduced globally!
      * another case is known with "foreach" --> here you can use "foreach (int i; int list {1,2,3})" --> note, typecasting is not used with paranthesis!

### Callbacks ###

bug fix in [r2116](https://code.google.com/p/toped/source/detail?r=2116) (fixes [issue 137](https://code.google.com/p/toped/issues/detail?id=137))
fully introduced in [r1939](https://code.google.com/p/toped/source/detail?r=1939) (fixes [issue 105](https://code.google.com/p/toped/issues/detail?id=105)).
fixes in [r1947](https://code.google.com/p/toped/source/detail?r=1947)

see also tll/callbackTest.tll

2 types of the usage of callback functions

```
//anonymous
void funcArguFuncB(int a, int b, callback int (int,int) unifunc)
{
   int result = unifunc(a,b);
   string buffer = sprintf ("%d fparam %d is %d", a, b, result);
   printf ("[%s] \n",buffer);
}

funcArguFuncB(2,3,&simpleAdd);
funcArguFuncB(2,3,&simpleSub);
```

```
// predefined
/* The callback type definition follows the function definition syntax
 * with two differencies:
 * - "callback" keyword marks the start of the definition
 * - the argument list is anonymous i.e. just a list of argument types
   callback <return_type> <functypename> ([<argument> [, <argument> [...]]])
*/
// define a callback type unifunctype ...
callback int unifunctype(int,int);

void funcArguFuncA(int a, int b, unifunctype unifunc)
{
   int result = unifunc(a,b);
   string buffer = sprintf ("%d fparam %d is %d", a, b, result);
   printf ("[%s] \n",buffer);
}

funcArguFuncA(2,3,@simpleAdd);
funcArguFuncA(2,3,@simpleSub);
```

To handle callback functions like variables you may use following definitions
```
// or define directly a variable unifunc of anonymous callback type
callback int (int,int) unifuncB = @simpleAdd;

// ... and a variable unifunc of that type
unifunctype unifuncA = @simpleAdd;
```



## Tell-functions ##

### Deprecated ###

  * echo();  ... since rev1900 to rev1094 ([issue103](https://code.google.com/p/toped/issues/detail?id=103)); use printf() instead

### Setting Parametes ###

refer to tpd\_bidfunc/tpdf\_props.cpp in
function tellstdfunc::analyzeTopedParameters

```
 setparams({"ADJUST_TEXT_ORIENTATION", "<bool>"}); ("true" .. each text aligned equally, "false" .. aligned to reference)
 setparams({"SELECT_TEXT_FONT", "<font>"}); ("Arial Normal 1" [Default], "Courier Normal 1", "Crystal 1", "Techno 0", "Techno 1", "Times New Roman 1")
 setparams({"CELL_VIEW_DEPTH", "<int>"});
 setparams({"CELL_DEPTH_ALPHA_EBB", "<real>"});
 setparams({"MIN_VISUAL_AREA", "<real>"});
 setparams({"GRC_BLINK_PERIOD", "<real>"});
 setparams({"HIGHLIGHT_ON_HOVER", "<bool>"});
 setparams({"UNDO_DEPTH", "<int>"}); 
```

