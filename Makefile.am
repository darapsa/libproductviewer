lib_LTLIBRARIES = libproductviewer.la
libproductviewer_la_SOURCES = viewer.c converter.c
if APPLE
libproductviewer_la_CPPFLAGS = -I${prefix}/System/Library/Frameworks/OpenGLES.framework/Headers
else
libproductviewer_la_CPPFLAGS = -I${includedir}
endif
include_HEADERS = productviewer.h
pkgdata_DATA = productviewer/product.vert \
	       productviewer/product.frag
