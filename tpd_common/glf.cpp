/*
==============================================================================
|  GLF Library
|  Version 1.4
| 
|  Author: Roman Podobedov
|  Email: romka@ut.ee
|  WEB: http://romka.demonews.com
|  Release Date: 18 May 2001
|  
|  Copyright (C) 2000-2001, Romka Graphics
|  This library is freely distributable without any license or permissions
|  for non-commercial usage. You can use this library in any non-commercial
|  program. In each program, where You use this library You should keep 
|  this header (author name and coordinates)!
|  For commercial usage, please contact me: romka@ut.ee
==============================================================================
*/

#include "tpdph.h"
#if defined (WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include "glf.h"

/* ---------- Definitions and structures ----- */

#define MAX_FONTS 256

/* Color structure */
struct color
{
   float r, g, b, a;
};

/* One symbol of font */
struct one_symbol
{
   unsigned char vertexs; /* Number of vertexs         */
   unsigned char facets;  /* Number of facets          */
   unsigned char lines;   /* Number of lines in symbol */

   float *vdata;          /* Pointer to Vertex data    */
   unsigned char *fdata;  /* Pointer to Facets data    */
   unsigned char *ldata;  /* Pointer to Line data      */

   float leftx;           /* Smaller x coordinate      */
   float rightx;          /* Right x coordinate        */
   float topy;            /* Top y coordinate          */
   float bottomy;         /* Bottom y coordinate       */
};

/* Font structure */
struct glf_font
{
   char font_name [97];
   unsigned char sym_total;          /* Total symbols in font */
   struct one_symbol *symbols[256];  /* Pointers to symbols   */
};

/* ------------- Main variables -------------- */

static float SymbolDist = 0.1f;  /* Distance between symbols (Variable constant) */
static float SymbolDepth = 0.2f; /* Symbol Depth in 3D space (Variable constant) */
static float SpaceSize = 2.0f;   /* Space size (Variable constant)               */
static float RotateAngle = 0.0f; /* Rotate angle for string (vector font) */
static float RotateAngleB = 0.0f; /* Rotate angle for string (bitmap font) */

/* Array of font pointers, up to MAX_FONTS fonts can be loaded at once */
/* if (fonts[i] == NULL) then this font is not present in memory */

static struct   glf_font *fonts[MAX_FONTS];
static int      curfont;         /* Index of current font pointer */
static char      ap = GLF_CENTER;   /* Anchor point */
static GLboolean m_string_center;   /* String centering (vector fonts) */
static GLboolean m_bitmap_string_center;   /* String centering (bitmap fonts) */

static GLuint   m_direction;   /* String direction (vector fonts) */

static char      console_msg = GLF_NO;
static char      texturing = GLF_NO;
static char      contouring = GLF_NO;
static char    filling = GLF_YES;
static struct   color contouring_color;

/* Console mode variables */
static int   conWidth, conHeight;   /* Console width and height */
static int   conx = 0, cony = 0;      /* Console current X and Y */
static char   *conData;            /* Console data */
static int   conFont;            /* Console font */
static char   conCursor = GLF_NO;      /* Console cursor Enabled|Disabled */
static int   conCursorBlink;         /* Console cursor blink rate */
static int   conCursorCount;         /* Console cursor blink counter */
static char   conCursorMode = GLF_NO; /* Console Cursor mode (on/off screen) */

/* ----------- Variables for bitmap font ------------- */
#define SEEK_SET_POS 4195

struct coord_rect
{
   float x, y, width, height;
};

struct widths
{
   float *width;
};

/* Constants */
//static float sym_space=0.001f;

/* One font variables */
static int    bmf_curfont;
static char     bmf_in_use[MAX_FONTS]; /* 1 - if font is used, 0 - otherwise */
static GLfloat m_max_height[MAX_FONTS]; /* Maximal height of each font */


/* Initialization of GLF library, should be called before use of library */
void glfInit()
{
   int i;

   for (i=0; i<MAX_FONTS; i++)
   {
      fonts[i] = NULL;
      bmf_in_use[i] = 0;
      m_max_height[i] = 0;
   }
   
   curfont = -1;
   bmf_curfont = -1;
   console_msg = GLF_NO;
   ap = GLF_CENTER;      /* Set anchor point to center of each symbol */
   texturing = GLF_NO;      /* By default texturing is NOT Enabled */
   contouring = GLF_NO;   /* By default contouring is NOT Enabled */
   filling = GLF_YES;   /* By default fill is Enabled */
   memset(&contouring_color, 0, sizeof(struct color));
   conData = NULL;
   glfSetConsoleParam(40, 20);
   glfConsoleClear();
   glfEnable(GLF_CONSOLE_CURSOR);
   glfSetCursorBlinkRate(10);
   glfStringCentering(GL_FALSE);
   glfBitmapStringCentering(GL_FALSE);
   glfStringDirection(GLF_LEFT);
}

/* Closing library usage */
void glfClose()
{
   int i;
   
   free(conData);

   for (i=0; i<MAX_FONTS; i++) glfUnloadFontD(i);
//   for (i=0; i<MAX_FONTS; i++) glfUnloadBMFFontD(i);
}

/* 
|   This function check that this machine is little endian
|   Return value:   1 - little endian
|               0 - big endian
*/
static int LittleEndian()
{
   int endianTester = 1;
   char *endianChar = 0;

   endianChar = (char *)&endianTester;
   if (*endianChar) return 1;
   return 0;
}


/*
   --------------------------------------------------------------------------------- 
   ------------------------ Work with vector fonts --------------------------------- 
   --------------------------------------------------------------------------------- 
*/

/* 
|   This function read font file and store information in memory
|   Return: GLF_OK - if all OK
|   Return: GLF_ERROR - if any error
*/
static int ReadFont(const char *font_name, struct glf_font *glff)
{

   FILE *fontf;
   char buffer[64];
   int i, j;
   unsigned char temp, code, verts, fcets, lns;
   float tempfx, tempfy;
   unsigned char *tp;
   int LEndian; /* True if little endian machine */

   fontf = fopen(font_name, "rb");
   if (fontf == NULL) return GLF_ERROR;

   fread(buffer, 3, 1, fontf);
   buffer[3] = 0;
   if (strcmp(buffer, "GLF"))
   {
      /* If header is not "GLF" */
      if (console_msg) 
         printf("Error reading font file: incorrect file format\n");
      fclose(fontf);
      return GLF_ERROR;
   }

   /* Check for machine type */
   LEndian = LittleEndian();

   fread(glff->font_name, 96, 1, fontf);
   glff->font_name[96] = 0;

   fread(&glff->sym_total, 1, 1, fontf); /* Read total symbols in font */

   for (i=0; i<MAX_FONTS; i++) glff->symbols[i] = NULL;

   for (i=0; i<28; i++) fread(&temp, 1, 1, fontf);  /* Read unused data */

   /* Now start to read font data */
  
   for (i=0; i<glff->sym_total; i++)
   {
      fread(&code, 1, 1, fontf);  /* Read symbol code   */
      fread(&verts, 1, 1, fontf); /* Read vertexs count */
      fread(&fcets, 1, 1, fontf); /* Read facets count  */
      fread(&lns, 1, 1, fontf);   /* Read lines count   */

      if (glff->symbols[code] != NULL)
      {
         if (console_msg) 
            printf("Error reading font file: encountered symbols in font\n");
         fclose(fontf);
         return GLF_ERROR;
      }

      glff->symbols[code] = (struct one_symbol *)malloc(sizeof(struct one_symbol));
      glff->symbols[code]->vdata = (float *)malloc(8*verts);
      glff->symbols[code]->fdata = (unsigned char *)malloc(3*fcets);
      glff->symbols[code]->ldata = (unsigned char *)malloc(lns);

      glff->symbols[code]->vertexs = verts;
      glff->symbols[code]->facets = fcets;
      glff->symbols[code]->lines = lns;

      /* Read vertexs data */
      glff->symbols[code]->leftx = 10;
      glff->symbols[code]->rightx = -10;
      glff->symbols[code]->topy = 10;
      glff->symbols[code]->bottomy = -10;

      for (j=0; j<verts; j++)
      {
         fread(&tempfx, 4, 1, fontf);
         fread(&tempfy, 4, 1, fontf);
          
         /* If machine is bigendian -> swap low and high words in
         tempfx and tempfy */
         if (!LEndian)
         {
            tp = (unsigned char *)&tempfx;
            temp = tp[0]; tp[0] = tp[3]; tp[3] = temp;
            temp = tp[1]; tp[1] = tp[2]; tp[2] = temp;
            tp = (unsigned char *)&tempfy;
            temp = tp[0]; tp[0] = tp[3]; tp[3] = temp;
            temp = tp[1]; tp[1] = tp[2]; tp[2] = temp;
         }
         glff->symbols[code]->vdata[j*2] = tempfx;
         glff->symbols[code]->vdata[j*2+1] = tempfy;

         if (tempfx < glff->symbols[code]->leftx) glff->symbols[code]->leftx = tempfx;
         if (tempfx > glff->symbols[code]->rightx) glff->symbols[code]->rightx = tempfx;
         if (tempfy < glff->symbols[code]->topy) glff->symbols[code]->topy = tempfy;
         if (tempfy > glff->symbols[code]->bottomy) glff->symbols[code]->bottomy = tempfy;
      }
      for (j=0; j<fcets; j++)
         fread(&glff->symbols[code]->fdata[j*3], 3, 1, fontf);
      for (j=0; j<lns; j++)
         fread(&glff->symbols[code]->ldata[j], 1, 1, fontf);
   }
   fclose(fontf);
   return GLF_OK;
}


/* 
| Function loads font to memory from file
| Return value: GLF_ERROR  - if error
|               >=0 - returned font descriptor (load success)
*/
int glfLoadFont(const char *font_name)
{
   int i;
   char flag; /* Temporary flag */

   /* First we find free font descriptor */
   flag = 0; /* Descriptor not found yet */
   for (i=0; i<MAX_FONTS; i++)
      if (fonts[i] == NULL)
      {
         /* Initialize this font */
         fonts[i] = (struct glf_font *)malloc(sizeof(struct glf_font));
         flag = 1;
         break;
      }

   if (!flag) return GLF_ERROR; /* Free font not found */
   if (ReadFont(font_name, fonts[i]) == GLF_OK)
   {
      curfont = i; /* Set curfont to just loaded font */
      return i;
   }
  
   if (fonts[i] != NULL)
   {
      free(fonts[i]);
      fonts[i] = NULL;
   }
   return GLF_ERROR;
}

/* 
| Unload current font from memory
| Return value: GLF_OK  - if all OK
|               GLF_ERROR -  if error
*/
int glfUnloadFont()
{
   int i;
  
   if ((curfont<0) || (fonts[curfont] == NULL)) return GLF_ERROR;
  
   for (i=0; i<256; i++)
   {
      if (fonts[curfont]->symbols[i] != NULL)
      {
         free(fonts[curfont]->symbols[i]->vdata);
         free(fonts[curfont]->symbols[i]->fdata);
         free(fonts[curfont]->symbols[i]->ldata);
         free(fonts[curfont]->symbols[i]);
      }
   }

   free(fonts[curfont]);
   fonts[curfont] = NULL;
   curfont = -1;
   return GLF_OK;
}

/* Unload font by font descriptor */
int glfUnloadFontD(int font_descriptor)
{
   int temp;

   if ((font_descriptor < 0) || (fonts[font_descriptor] == NULL)) return GLF_ERROR;

   temp = curfont;
   curfont = font_descriptor;
   glfUnloadFont();
   if (temp != font_descriptor) curfont = temp;
   else curfont = -1;
   return GLF_OK;
}

void glfDrawWiredSymbol(char s)
{
   int i, cur_line;
   float *tvp; /* temporary vertex pointer */
   float x, y;

   if ((curfont < 0) || (fonts[curfont] == NULL)) return;
   one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char) s];
   if (the_symbol == NULL) return;

   glBegin(GL_LINE_LOOP);
   tvp = the_symbol->vdata;
   cur_line = 0;
   for (i=0; i<the_symbol->vertexs; i++)
   {
      x = *tvp++;
      y = *tvp++;
      glVertex2f(x, y);
      if (the_symbol->ldata[cur_line] == i)
      {
         glEnd();
         cur_line++;
         if (cur_line < the_symbol->lines) glBegin(GL_LINE_LOOP);
         else break; /* No more lines */
      }
   }
}


// void (char s)
// {
//    int i, cur_line;
//    float *tvp; /* temporary vertex pointer */
//    float x, y;
//   
//    if ((curfont < 0) || (fonts[curfont] == NULL)) return;
//    if (fonts[curfont]->symbols[s] == NULL) return;
// 
//    glBegin(GL_LINE_LOOP);
//    tvp = fonts[curfont]->symbols[s]->vdata;
//    cur_line = 0;
//    for (i=0; i<fonts[curfont]->symbols[s]->vertexs; i++)
//    {
//       x = *tvp++;
//       y = *tvp++;
//       glVertex2f(x, y);
//       if (fonts[curfont]->symbols[s]->ldata[cur_line] == i)
//       {
//          glEnd();
//          cur_line++;
//          if (cur_line < fonts[curfont]->symbols[s]->lines) glBegin(GL_LINE_LOOP);
//          else break; /* No more lines */
//       }
//    }
//    /* Draw fill, if enabled */
//    if (filling == GLF_YES)
//       glfDrawSolidSymbol(s);
// }

/* Draw wired symbol by font_descriptor */
void glfDrawWiredSymbolF(int font_descriptor, char s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   glfDrawWiredSymbol(s);
   curfont = temp;
}

static void DrawString(const char *s, void (*funct) (char s))
{
   int i;
   float sda, sdb;
   float distance;

   if (!s) return;
   if (!*s) return;
   if (curfont == -1) return;

   /* Calculate correction (if string centering enabled) */
   if (m_string_center)
   {
      distance = 0;
      for (i=0; i<(int)strlen(s); i++)
      {
         one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)(s[i])];
         if ((the_symbol == NULL) || (s[i] == ' '))
         {
            if (m_direction == GLF_LEFT || m_direction == GLF_UP) distance += SpaceSize;
            else distance -= SpaceSize;
         }
         else
         {
            if (i < ((int)strlen(s)-1))
            {
               if (s[i+1] == ' ')
               {
                  if (m_direction == GLF_LEFT || m_direction == GLF_UP) distance += SymbolDist;
                  else distance -= SymbolDist;
               }
               else
               {
                  one_symbol* the_next_symbol = fonts[curfont]->symbols[(unsigned char)(s[i+1])];
                  if (the_next_symbol == NULL) continue;
   
                  if (m_direction == GLF_LEFT || m_direction == GLF_RIGHT)
                  {
                     sda = (float)fabs(the_symbol->rightx);
                     sdb = (float)fabs(the_next_symbol->leftx);
                     if (m_direction == GLF_LEFT) distance += sda+sdb+SymbolDist;
                     else distance -= sda+sdb+SymbolDist;
                  }
                  else
                  {
                     sda = (float)fabs(the_symbol->topy);
                     sdb = (float)fabs(the_symbol->bottomy);
                     if (m_direction == GLF_DOWN) distance -= sda+sdb+SymbolDist;
                     else distance += sda+sdb+SymbolDist;
                  }
               }
            }
         }
      }
   }

   glPushMatrix();
   
   /* Rotate if needed */
   if (RotateAngle != 0.0f) glRotatef(RotateAngle, 0, 0, 1);

   /* Correct string position */
//    if (m_string_center)
//    {
//       switch (m_direction)
//       {
//          case GLF_LEFT : glTranslatef(-distance/2, 0, 0); break;
//          case GLF_RIGHT : glTranslatef(distance/2, 0, 0); break;
//          case GLF_UP : glTranslatef(0, distance/2, 0); break;
//          case GLF_DOWN : glTranslatef(0, -distance/2, 0); break;
//       }
//    }
//    else if (s[0] != ' ')
//    {
//       switch (m_direction)
//       {
//          case GLF_LEFT : glTranslatef(-(1-(float)fabs(fonts[curfont]->symbols[s[0]]->leftx)), 0, 0); break;
//          case GLF_RIGHT : glTranslatef((1-(float)fabs(fonts[curfont]->symbols[s[0]]->rightx)), 0, 0); break;
//          case GLF_UP : glTranslatef(0, (1-(float)fabs(fonts[curfont]->symbols[s[0]]->topy)), 0); break;
//          case GLF_DOWN : glTranslatef(0, -(1-(float)fabs(fonts[curfont]->symbols[s[0]]->bottomy)), 0); break;
//       }
//    }

   /* Start to draw our string */
   for (i=0; i<(int)strlen(s); i++)
   {
      if (s[i] != ' ') (*funct) (s[i]);
      one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)(s[i])];
      if ((the_symbol == NULL) || (s[i] == ' '))
      {
         switch (m_direction)
         {
            case GLF_LEFT : glTranslatef(SpaceSize, 0, 0); break;
            case GLF_RIGHT : glTranslatef(-SpaceSize, 0, 0); break;
            case GLF_UP : glTranslatef(0, SpaceSize, 0); break;
            case GLF_DOWN : glTranslatef(0, -SpaceSize, 0); break;
         }
      }
      else
      {
         if (i < ((int)strlen(s)-1))
         {
            if (s[i+1] == ' ')
            {
               switch (m_direction)
               {
                  case GLF_LEFT : glTranslatef(SymbolDist, 0, 0); break;
                  case GLF_RIGHT : glTranslatef(-SymbolDist, 0, 0); break;
                  case GLF_UP : glTranslatef(0, SymbolDist, 0); break;
                  case GLF_DOWN : glTranslatef(0, -SymbolDist, 0); break;
               }
            }
            else
            {
               one_symbol* the_next_symbol = fonts[curfont]->symbols[(unsigned char)(s[i+1])];
               if (the_next_symbol == NULL) continue;
               if (m_direction == GLF_LEFT || m_direction == GLF_RIGHT)
               {
                  if (m_direction == GLF_LEFT)
                  {
                     sda = (float)fabs(the_symbol->rightx);
                     sdb = (float)fabs(the_next_symbol->leftx);
                  }
                  else
                  {
                     sda = (float)fabs(the_next_symbol->rightx);
                     sdb = (float)fabs(the_symbol->leftx);
                  }
                  if (m_direction == GLF_LEFT) glTranslatef(sda+sdb+SymbolDist, 0, 0);
                  else glTranslatef(-(sda+sdb+SymbolDist), 0, 0);
               }
               else
               {
                  if (m_direction == GLF_DOWN)
                  {
                     sda = (float)fabs(the_symbol->topy);
                     sdb = (float)fabs(the_next_symbol->bottomy);
                  }
                  else
                  {
                     sda = (float)fabs(the_next_symbol->topy);
                     sdb = (float)fabs(the_symbol->bottomy);
                  }
                  if (m_direction == GLF_DOWN) glTranslatef(0, -(sda+sdb+SymbolDist), 0);
                  else glTranslatef(0, sda+sdb+SymbolDist, 0);
               }
            }
         }
      }
   }
   glPopMatrix();
}

void glfDrawWiredString(const char *s)
{
   DrawString(s, &glfDrawWiredSymbol);
}


void glfDrawTopedString(const char *s, bool fill)
{
//   DrawString(s, &glfDrawTopedSymbol);
   int i;
   float sda, sdb;

   if ((!s) || (!*s) || (curfont == -1))
      return;
   else
   {
      void(* symbfunc) (char)     = (fill) ? &glfDrawSolidSymbol : &glfDrawWiredSymbol;
      glPushMatrix();
   
      /* Start to draw the string */
      for (i=0; i<(int)strlen(s); i++)
      {
         // draw
         if (s[i] != ' ') (* symbfunc)(s[i]);
         //move the current position
         one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)(s[i])];
         if ((the_symbol == NULL) || (s[i] == ' '))
            glTranslatef(SpaceSize, 0, 0);
         else if (i < ((int)strlen(s)-1))
         {
            if (s[i+1] == ' ')
               glTranslatef(SymbolDist, 0, 0);
            else
            {
               one_symbol* the_next_symbol = fonts[curfont]->symbols[(unsigned char)(s[i+1])];
               if (the_next_symbol == NULL) continue;
               sda = (float)fabs(the_symbol->rightx);
               sdb = (float)fabs(the_next_symbol->leftx);
               glTranslatef(sda+sdb+SymbolDist, 0, 0);
            }
         }
      }
      glPopMatrix();}
}

/* Draw wired string by font_descriptor */
void glfDrawWiredStringF(int font_descriptor, char *s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   DrawString(s, &glfDrawWiredSymbol);
   curfont = temp;
}

void glfDrawSolidSymbol(char s)
{
   unsigned char *b; /* Face pointer   */
   float *vp;        /* Vertex pointer */
   int i, j;
   float x, y;
//   float temp_color[4];

   if ((curfont<0) || (fonts[curfont] == NULL)) return;
   one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)s];
   if (the_symbol == NULL) return;

   b = the_symbol->fdata;
   vp = the_symbol->vdata;

   glBegin(GL_TRIANGLES);
   for (i=0; i<the_symbol->facets; i++)
   {
      for (j=0; j<3; j++)
      {
         x = vp[*b*2];
         y = vp[*b*2+1];
         if (texturing == GLF_YES) glTexCoord2f((x+1)/2, (y+1)/2);
         glVertex2f(x, y);
         b++;
      }
   }
   glEnd();

   //   /* Draw contour, if enabled */
   //if (contouring == GLF_YES)
   //{
//      glGetFloatv(GL_CURRENT_COLOR, temp_color);
//      glColor4f(contouring_color.r, contouring_color.g, contouring_color.b, contouring_color.a);
//      glfDrawWiredSymbol(s);
//      glColor4fv(temp_color);
   //}
}

/* Draw solid symbol by font_descriptor */
void glfDrawSolidSymbolF(int font_descriptor, char s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   glfDrawSolidSymbol(s);
   curfont = temp;
}

void glfDrawSolidString(const char *s)
{
   DrawString(s, &glfDrawSolidSymbol);
}

/* Draw solid string by font_descriptor */
void glfDrawSolidStringF(int font_descriptor, char *s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   DrawString(s, &glfDrawSolidSymbol);
   curfont = temp;
}


/* ------------ 3D Wired text drawing ---------------------- */

void glfDraw3DWiredSymbol(char s)
{
   int i, cur_line;
   float *tvp; /* temp vertex pointer */
   float x, y;
  
   if ((curfont<0) || (fonts[curfont] == NULL)) return;
   if (fonts[curfont]->symbols[(int)s] == NULL) return;

   /* Draw front symbol */
   glBegin(GL_LINE_LOOP);
   one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)s];
   tvp = the_symbol->vdata;
   cur_line = 0;
   for (i=0; i< the_symbol->vertexs; i++)
   {
      x = *tvp;
      tvp++;
      y = *tvp;
      tvp++;
      glVertex3f(x, y, 1);
      if (the_symbol->ldata[cur_line] == i)
      {
         glEnd();
         cur_line++;
         if (cur_line < the_symbol->lines) glBegin(GL_LINE_LOOP);
         else break; /* No more lines */
      }
   }

   /* Draw back symbol */
   glBegin(GL_LINE_LOOP);
   tvp = the_symbol->vdata;
   cur_line = 0;
   for (i=0; i < the_symbol->vertexs; i++)
   {
      x = *tvp;
       tvp++;
      y = *tvp;
      tvp++;
      glVertex3f(x, y, 1+SymbolDepth);
      if (the_symbol->ldata[cur_line] == i)
      {
         glEnd();
         cur_line++;
         if (cur_line < the_symbol->lines) glBegin(GL_LINE_LOOP);
         else break; /* No more lines */
      }
   }

   /* Draw lines between back and front symbols */
   glBegin(GL_LINES);
   tvp = the_symbol->vdata;
   for (i=0; i<the_symbol->vertexs; i++)
   {
      x = *tvp;
      tvp++;
      y = *tvp;
      tvp++;
      glVertex3f(x, y, 1);
      glVertex3f(x, y, 1+SymbolDepth);
   }
   glEnd();
}

/* Draw 3D wired symbol by font_descriptor */
void glfDraw3DWiredSymbolF(int font_descriptor, char s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   glfDraw3DWiredSymbol(s);
   curfont = temp;
}

void glfDraw3DWiredString(char *s)
{
   DrawString(s, &glfDraw3DWiredSymbol);
}

/* Draw 3D wired string by font_descriptor */
void glfDraw3DWiredStringF(int font_descriptor, char *s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   DrawString(s, &glfDraw3DWiredSymbol);
   curfont = temp;
}

/* ------------ 3D Solid text drawing ---------------------- */

void glfDraw3DSolidSymbol(char s)
{
   int i, j, cur_line, flag;
   float x, y, bx, by;
   unsigned char *b; /* Face pointer   */
   float *vp;        /* Vertex pointer */
   float *tvp;       /* temp vertex pointer */
   float temp_color[4];
   GLboolean light_temp;
  
   if ((curfont<0) || (fonts[curfont] == NULL)) return;
   if (fonts[curfont]->symbols[(int)s] == NULL) return;
   one_symbol* the_symbol = fonts[curfont]->symbols[(unsigned char)s];
   b = the_symbol->fdata;
   vp = the_symbol->vdata;

   glBegin(GL_TRIANGLES);
   glNormal3f(0, 0, 1);
   for (i=0; i<the_symbol->facets; i++)
   {
      b += 2;
      for (j=0; j<3; j++)
      {
         x = vp[*b*2];
         y = vp[*b*2+1];
         glVertex3f(x, y, 1+SymbolDepth);
         b--;
      }
      b += 4;
   }
   glEnd();

   b = the_symbol->fdata;
   vp = the_symbol->vdata;
  
   glBegin(GL_TRIANGLES);
   glNormal3f(0, 0, -1);
   for (i=0; i<the_symbol->facets; i++)
   {
      for (j=0; j<3; j++)
      {
         x = vp[*b*2];
         y = vp[*b*2+1];
         glVertex3f(x, y, 1);
         b++;
      }
   }
   glEnd();

   flag = 0;
   glBegin(GL_QUAD_STRIP);
   tvp = the_symbol->vdata;
   cur_line = 0;
   for (i=0; i<the_symbol->vertexs; i++)
   {
      x = *tvp;
      tvp++;
      y = *tvp;
      tvp++;
      if (!flag)
      {
         bx = x;
         by = y;
         flag = 1;
      }
      glNormal3f(x, y, 0);
      glVertex3f(x, y, 1);
      glVertex3f(x, y, 1+SymbolDepth);
      if (the_symbol->ldata[cur_line] == i)
      {
         glVertex3f(bx, by, 1);
         glVertex3f(bx, by, 1+SymbolDepth);
         flag = 0;
         glEnd();
         cur_line++;
         if (cur_line < the_symbol->lines) glBegin(GL_QUAD_STRIP);
         else break; /* No more lines */
      }
   }

   /* Draw contour, if enabled */
   if (contouring == GLF_YES)
   {
      glGetBooleanv(GL_LIGHTING, &light_temp);
      glDisable(GL_LIGHTING);
      glGetFloatv(GL_CURRENT_COLOR, temp_color);
      glColor4f(contouring_color.r, contouring_color.g, contouring_color.b, contouring_color.a);
      glfDraw3DWiredSymbol(s);
      glColor4fv(temp_color);
      if (light_temp) glEnable(GL_LIGHTING);
   }
}

/* Draw 3D solid symbol by font_descriptor */
void glfDraw3DSolidSymbolF(int font_descriptor, char s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   glfDraw3DSolidSymbol(s);
   curfont = temp;
}

void glfDraw3DSolidString(char *s)
{
   DrawString(s, &glfDraw3DSolidSymbol);
}

/* Draw 3D solid string by font_descriptor */
void glfDraw3DSolidStringF(int font_descriptor, char *s)
{
   int temp;

   temp = curfont;
   curfont = font_descriptor;
   DrawString(s, &glfDraw3DSolidSymbol);
   curfont = temp;
}

/* Get the size a string will have on screen */
void glfGetStringBoundsF(int fd, const char *s, float *minx, float *miny, float *maxx, float *maxy)
{
   struct glf_font *font;
   int i;
   float sda, sdb, cw = 0, minxx = 10;
   float top = 10, bottom = -10;

   if (fd < 0 || fd > (MAX_FONTS-1)) return;
   font = fonts[fd];

   if (font == NULL) return;

   if (font->symbols[(unsigned char)(s[0])])
      minxx = font->symbols[(unsigned char)(s[0])]->leftx;
   else
      minxx = 0.0;

   for (i=0; i<(int)strlen(s); i++)
   {
      if ((font->symbols[(unsigned char)(s[i])] == NULL) || (s[i] == ' '))
         cw += SpaceSize;
      else
      {
         sdb = -font->symbols[(int)s[i]]->leftx;
         sda = font->symbols[(int)s[i]]->rightx;

         cw += sda+sdb+SymbolDist;

         /* Update top/bottom bounds */
         if (font->symbols[(int)s[i]]->bottomy > bottom)
            bottom = font->symbols[(int)s[i]]->bottomy;

         if (font->symbols[(int)s[i]]->topy < top)
            top = font->symbols[(int)s[i]]->topy;
      }
   }

   cw += minxx;

   if ((maxx) && (maxy))
   {
      *maxx = cw;
      *maxy = bottom;
   }

   if ((minx) && (miny))
   {
      *minx = minxx;
      *miny = top;
   }
}

void glfGetStringBounds(const char *s, float *minx, float *miny, float *maxx, float *maxy)
{
   glfGetStringBoundsF(curfont, s, minx, miny, maxx, maxy);
}

void glfSetSymbolSpace(float sp)
{
   SymbolDist = sp;
}

float glfGetSymbolSpace()
{
   return SymbolDist;
}

void glfSetSpaceSize(float sp)
{
   SpaceSize = sp;
}

float glfGetSpaceSize()
{
   return SpaceSize;
}

void glfSetSymbolDepth(float dpth)
{
   SymbolDepth = dpth;
}

float glfGetSymbolDepth()
{
   return SymbolDepth;
}

int glfSetCurrentFont(int Font_Descriptor)
{
   if ((Font_Descriptor < 0) || (fonts[Font_Descriptor] == NULL)) return GLF_ERROR;
  
   curfont = Font_Descriptor;
   return GLF_OK;
}

int glfGetCurrentFont()
{
   return curfont;
}

void glfSetAnchorPoint(int anchp)
{
   if ((anchp >= GLF_LEFT_UP) && (anchp <= GLF_RIGHT_DOWN))
      ap = anchp;
}

void glfSetContourColor(float r, float g, float b, float a)
{
   contouring_color.r = r;
   contouring_color.g = g;
   contouring_color.b = b;
   contouring_color.a = a;
}

void glfEnable(int what)
{
   switch (what)
   {
      case GLF_CONSOLE_MESSAGES: console_msg = GLF_YES; break;
      case GLF_TEXTURING: texturing = GLF_YES; break;
      case GLF_CONSOLE_CURSOR: conCursor = GLF_YES; break;
      case GLF_CONTOURING: contouring = GLF_YES; break;
//      case GLF_FILLING: filling = GLF_YES; break;
   }
}

void glfDisable(int what)
{
   switch (what)
   {
      case GLF_CONSOLE_MESSAGES: console_msg = GLF_NO; break;
      case GLF_TEXTURING: texturing = GLF_NO; break;
      case GLF_CONSOLE_CURSOR: conCursor = GLF_NO; break;
      case GLF_CONTOURING: contouring = GLF_NO; break;
//      case GLF_FILLING: filling = GLF_NO; break;
   }
}

/* ---------------- Console functions ---------------------- */

void glfSetConsoleParam(int width, int height)
{
   if (conData) free(conData);

   conWidth = width;
   conHeight = height;
   conData = (char *)malloc(width*height);
   glfConsoleClear();
}

int glfSetConsoleFont(int Font_Descriptor)
{
   if ((Font_Descriptor < 0) || (fonts[Font_Descriptor] == NULL)) return GLF_ERROR;
  
   conFont = Font_Descriptor;
   return GLF_OK;
}

void glfConsoleClear()
{
   memset(conData, 0, conWidth*conHeight);
   conx = 0;
   cony = 0;
}

void glfPrint(char *s, int lenght)
{
   int i;

   for (i=0; i<lenght; i++)
   {
      if (s[i] > 31)
      {
         conData[cony*conWidth+conx] = s[i];
         conx++;
      } else
      if (s[i] == '\n') conx = conWidth;
      if (conx >= conWidth)
      {
         conx = 0;
         cony++;
         if (cony >= conHeight)
         {
            /* Shift all console contents up */
            memcpy(conData, &conData[conWidth], conWidth*(conHeight-1));
            /* Fill bottom line by spaces */
            memset(&conData[conWidth*(conHeight-1)], 0, conWidth);
            cony = conHeight-1;
         }
      }
   }
}

void glfPrintString(char *s)
{
   glfPrint(s, strlen(s));
}

void glfPrintChar(char s)
{
   glfPrint(&s, 1);
}

void glfConsoleDraw()
{
   int i, j;
   char s[512];

   for (i=0; i<conHeight; i++)
   {
      memcpy(s, &conData[i*conWidth], conWidth);
      s[conWidth] = 0;
      if ((conCursor == GLF_YES) && (i == cony))
      {
         conCursorCount--;
         if (conCursorCount < 0)
         {
            conCursorCount = conCursorBlink;
            if (conCursorMode == GLF_YES) conCursorMode = GLF_NO;
            else conCursorMode = GLF_YES;
         }

         if (conCursorMode == GLF_YES)
         for (j=0; j<conWidth; j++)
         {
            if (!s[j])
            {
               s[j] = '_'; /* Draw cursor */
               s[j+1] = 0;
               break;
            }
         }
      }
      glfDrawSolidStringF(conFont, s);
      glTranslatef(0, -2, 0);
   }
}

void glfSetCursorBlinkRate(int Rate)
{
   if (Rate > 0)
   {
      conCursorBlink = Rate;
      conCursorCount = Rate;
      conCursorMode = GLF_YES;
   }
}

/* Set string centering for vector fonts */
void glfStringCentering(GLboolean center)
{
   m_string_center = center;
}

/* String direction for vector font (GLF_LEFT, GLF_RIGHT, GLF_UP, GLF_DOWN) */
/* GLF_LEFT by default */
void glfStringDirection(GLuint direction)
{
   if (direction == GLF_LEFT || direction == GLF_RIGHT ||
      direction == GLF_UP || direction == GLF_DOWN) m_direction = direction;
}

/* Get current text direction */
GLuint glfGetStringDirection()
{
   return m_direction;
}


/* Get string centering for vector fonts */
GLboolean glfGetStringCentering()
{
   return m_string_center;
}

/* Set rotate angle for vector fonts */
void glfSetRotateAngle(float angle)
{
   RotateAngle = angle;
}


/*
   --------------------------------------------------------------------------------- 
   ------------------------ Work with bitmapped fonts ------------------------------ 
   --------------------------------------------------------------------------------- 
*/

/* Set string centering for bitmap fonts */
void glfBitmapStringCentering(GLboolean center)
{
   m_bitmap_string_center = center;
}

/* Set string centering for bitmap fonts */
GLboolean glfBitmapGetStringCentering()
{
   return m_bitmap_string_center;
}

/* Set rotate angle for bitmap fonts */
void glfSetBRotateAngle(float angle)
{
   RotateAngleB = angle;
}
