lib_LTLIBRARIES = libtpd_common.la

noinst_HEADERS =                                                              \
                 tpdph.h                                                      \
                 avl_def.h                                                    \
                 avl.h                                                        \
                 polycross.h                                                  \
                 tuidefs.h

libtpd_common_la_HEADERS =                                                    \
                 ttt.h                                                        \
                 outbox.h                                                     \
                 tedbac.h                                                     \
                 MemTrack.h

libtpd_common_la_SOURCES =                                                    \
                 tpdph.cpp                                                    \
                 avl.cpp                                                      \
                 outbox.cpp                                                   \
                 polycross.cpp                                                \
                 tedbac.cpp                                                   \
                 ttt.cpp                                                      \
                 MemTrack.cpp

###############################################################################
libtpd_common_ladir=$(includedir)/toped_$(PACKAGE_VERSION)/tpd

libtpd_common_la_LDFLAGS = -no-undefined