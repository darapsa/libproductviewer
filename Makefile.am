lib_LTLIBRARIES = libproductviewer.la
libproductviewer_la_SOURCES = viewer.c converter.c
libproductviewer_la_CFLAGS = -I${prefix}/include
if APPLE
libproductviewer_la_CFLAGS += -I${prefix}/System/Library/Frameworks/OpenGLES.framework/Headers
endif
if WASM
libproductviewer_la_LDFLAGS = -static
endif
if IOS
libproductviewer_la_LDFLAGS = -static
endif
include_HEADERS = productviewer.h
