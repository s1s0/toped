#ifdef WIN32
   #include "wx/wxprec.h"               // wxWidgets precompiled / standard headers
   #include "wx/notebook.h"             // other headers I need
   #include "wx/statline.h"
   #include "wx/tglbtn.h"

   // debug memory allocation enhancement (see next tip)
   #ifdef _DEBUG
   #include <crtdbg.h>
   #define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
   #else
   #define DEBUG_NEW new
   #endif

   #pragma warning (disable:4786)
   #include <map>
   #include <vector>
#else
   #ifdef DB_MEMORY_TRACE
      #include "MemTrack.h"
      #define DEBUG_NEW MEMTRACK_NEW
   #else
      #define DEBUG_NEW new
   #endif
#endif
//using namespace std;
#define DBGL_CALL(fName, args...) \
   trend::dbgl_call((char*)__FILE__,__LINE__,(char*)(#fName), fName, args);

#define DBGL_CALL0(fName) \
   trend::dbgl_call((char*)__FILE__,__LINE__,(char*)(#fName), fName);
