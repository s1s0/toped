SET(lib_LTLIBRARIES tpd_bidfunc)
SET(libtpd_bidfunc_la_SOURCES tpdf_db.cpp tpdf_select.cpp datacenter.cpp
	tellibin.cpp tllf_list.cpp tpdf_add.cpp tpdf_cells.cpp tpdf_common.cpp 
	tpdf_edit.cpp tpdf_get.cpp tpdf_props.cpp tpdph.cpp drc_tenderer.cpp  )
SET(libtpd_bidfunc_la_HEADERS tpdf_common.h datacenter.h )


include_directories(../tpd_common ../tpd_DB  ../tpd_ifaces ../tpd_parser ${OPENGL_INCLUDE_DIR} ${glew_INCLUDE_DIR})

INCLUDE(${wxWidgets_USE_FILE})
add_library(${lib_LTLIBRARIES}  STATIC ${libtpd_bidfunc_la_SOURCES}) 