SUBDIRS_MAIN = ui                                         \
               tpd_common                                 \
               tpd_GL                                     \
               tpd_DB                                     \
               tpd_ifaces                                 \
               tpd_parser                                 \
               tpd_bidfunc                                \
               src                                        \
               fonts                                      \
               icons                                      \
               tll                                        \
               utils/cadence
if UTILS
   SUBDIRS =   $(SUBDIRS_MAIN)                 \
               utils/gds2vrml_v0.3/libgdsto3d  \
               utils/gds2vrml_v0.3/src
   top_gds2vrml_srcdir = $(top_srcdir)/utils/gds2vrml_v0.3
else
   SUBDIRS =   $(SUBDIRS_MAIN)
endif
