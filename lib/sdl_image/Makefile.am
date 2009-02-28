INCLUDES = -I$(top_srcdir)
CFLAGS = @CFLAGS@ @SDL_CFLAGS@ -DLOAD_BMP -DLOAD_GIF -DLOAD_JPG -DLOAD_LBM -DLOAD_PCX -DLOAD_PNG -DLOAD_PNM -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libsdl_image.la

libsdl_image_la_SOURCES = \
	IMG.c \
	IMG_bmp.c \
	IMG_gif.c \
	IMG_jpg.c \
	IMG_lbm.c \
	IMG_pcx.c \
	IMG_png.c \
	IMG_pnm.c \
	IMG_tga.c \
	IMG_tif.c \
	IMG_xcf.c \
	IMG_xpm.c \
	IMG_xxx.c
