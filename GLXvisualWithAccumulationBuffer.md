<p align='right'> <b>08/10/2008</b>
</p>

---

# Toped on Fedora 9 #

### Abstract ###
If Toped crashes on startup - make sure you're using the latest released or packaged revision. If it still refuses to start, and reports something about the GLX visual:
  1. Check your GLX version (glxinfo)
  1. If it is prior to 1.3, update your /etc/X11/xorg.conf (see below) and try again.
  1. If it is 1.3 or newer - please report a [bug](https://developer.berlios.de/bugs/?group_id=5037).
If you're still curious - here is the full story


---

Toped rev 0.9 was released relatively at the same time as FC9 and was not tested on it. It was flagged first by the packagers that toped crashes on startup. The problem was not seen on FC8, neither on Ubuntu, nor on Windows. The problem was seen with ATI cards and certainly with Mesa only.

Initial investigations revealed some interesting things about FC9 distribution packages
  * rpm claimed that mesa-libGL-7.1 is installed, but on the [mesa home page](http://www.mesa3d.org/) the latest release at that time was 7.0.3
  * the situation with the [Xorg](http://www.x.org/wiki/) versions was similar. [Revision 7](https://code.google.com/p/toped/source/detail?r=7).4 was not released, but it was already somehow (at least partially) in the distribution.
  * The only available Radeon driver even today is still the GPL-ed one. ATi still haven't released anything for X 7.4
It appears to be quite a challenge to make a descent graphics with the above combination of libraries and the latest [wxWidgets](http://www.wxwidgets.org/).

### The GLX visual and the crash ###
Toped reqires some openGL features to work properly - for example double buffering and accumulation buffers. Those are rather regular features and are claimed to be implemented in all known openGL flavors. Indeed troubles with GLX window initialisation were never seen before in the project lifetime.
Toped doesn't initialise GLX by itself. Instead it is using the [wxGLCanvas](http://docs.wxwidgets.org/2.8.9/wx_wxglcanvas.html) class. The problem with this class was (and still is) that there is no way to find out whether a valid GLX visual had been obtained. In the debug version of the library you can still see wxWidgets reporting that GLX visual can't be obtained, but in the run version - it was simply crashing. I've reported it - see [Query for openGL capabilities](http://groups.google.com/group/comp.soft-sys.wxwindows/browse_thread/thread/2c204301f78a97c8/702a303ad22c22a4?hl=en&lnk=gst&q=Svilen#702a303ad22c22a4). It appeared that there were already some ideas how to update the class, but they haven't reach the mainstream release yet.
The only option left to prevent the crash was to implement a temporary GTK specific workaround. It was done and released as a [patch](http://prdownload.berlios.de/toped/release_0.9_patch_654_721.patch) to rev.0.9. Now the crash was prevented although not in the most elegant way, but the main problem was still there.

### What's wrong with the GLX? ###
The main suspect at the beginning was the ATI driver from DRI. The driver proves to be OK however - at least at this stage. It took me some time to distribute some "spam" on a number of development mailing lists, but finally thanks to some good people I've got the proper mailing list. It was rather exciting to realise that due to some recent updates in GLX, the accumulation buffers are not more a default feature -see [here](http://www.mail-archive.com/dri-devel@lists.sourceforge.net/msg35860.html). Fair enough! It was still a good news though. Here is the essence of the solution:
/etc/X11/xorg.conf
```
Section "ServerLayout"
      ...
       Option         "GlxVisuals" "all"
EndSection

```
OK, this finally made toped running on FC9, but it also rose more questions.

### Accumulation buffers - a deprecated feature ? ###
I'm certainly not a GLX expert, but there are generally two ways to obtain a drawable openGL window under X:
  * [glXChooseVisual](http://www.opengl.org/sdk/docs/man/xhtml/glXChooseVisual.xml) - this is the "traditional" one
  * [glXChooseFBConfig](http://www.opengl.org/sdk/docs/man/xhtml/glXChooseFBConfig.xml) - this is if frame buffers are to be used
Both of the methods are using virtually the same attribute lists (screen properties). Accumulation buffers are among those properties. In both cases. They might be differently implemented, nevertheles as a feature accumulation buffers seem to be pretty much alive.
Toped doesn't call the functions listed above directly. GLX visual is obtained using [wxGLCanvas](http://docs.wxwidgets.org/2.8.9/wx_wxglcanvas.html). Quick look in the wxWidgets code shows that the first fuction is called if GLX version is prior to 1.3. Frame buffers are used if GLX version is 1.3 or later. This is not a surprise - frame buffers were introduced in GLX version 1.3.
Here is the question then - with frame buffers we shouldn't need any additional options in the xorg.conf file. What is then the

### GLX version on FC9? ###
Here is what glxinfo utility reports on a standard FC9 installation with ATI videocard (see the date of the post at the top of the page)
```
GLX version: **1.2**
OpenGL vendor string: DRI R300 Project
OpenGL renderer string: Mesa DRI R300 20060815 x86/MMX/SSE2 TCL
OpenGL version string: 1.3 Mesa 7.1 rc1

```
That looks a bit ancient to me. But the good news is that it explains a lot!
It appears that the only option to obtain any kind of GLX visual is the "traditional" one. It appears that frame buffers are unusable. If that is so, then why the traditional implementation of accumulation buffers was optioned out by default? This looks to me as a **general distribution issue**.

Obviously I've spent too much time in the comfort of the vendor video drivers and not surprisingly I got spoiled. It was a time when a men was writing its own drivers...


---