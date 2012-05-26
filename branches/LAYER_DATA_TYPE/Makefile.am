SUBDIRS_MAIN = ui                                         \
               tpd_common                                 \
               tpd_DB                                     \
               tpd_ifaces                                 \
               tpd_parser                                 \
               tpd_bidfunc                                \
               src                                        \
               fonts                                      \
               icons                                      \
               tll                                        \
               tll/user_functions                         \
               tll/user_functions/config                  \
               tll/user_functions/devices                 \
               tll/user_functions/technology/             \
               tll/user_functions/technology/mosis_SCMOS_p2m6 \
               tll/user_functions/test/                   \
               tll/user_functions/general                 \
               tll/user_functions/general/examples        \
               tll/user_functions/tools                   \
               tll/user_functions/tools/logos             \
               tll/user_functions/tools/logos/examples    \
               utils/cadence
if UTILS
   SUBDIRS =   $(SUBDIRS_MAIN)                 \
               utils/gds2vrml_v0.3/libgdsto3d  \
               utils/gds2vrml_v0.3/src
   top_gds2vrml_srcdir = $(top_srcdir)/utils/gds2vrml_v0.3
else
   SUBDIRS =   $(SUBDIRS_MAIN)
endif
