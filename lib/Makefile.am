if BUILD_FLASH
FLASHDIR = flash
endif

if BUILD_GOOM
GOOMDIR = goom
endif

if BUILD_JAVASCRIPT
JAVASCRIPTDIR = javascript
endif

if BUILD_SDLGFX
SDLGFXDIR = sdl_gfx
endif

if BUILD_SDLTTF
SDLTTFDIR = sdl_ttf
endif

if BUILD_SDLIMAGE
SDLIMAGEDIR = sdl_image
endif

SUBDIRS = ccvt \
	slw              \
	$(SDLIMAGEDIR)   \
	$(SDLTTFDIR)     \
	$(SDLGFXDIR)     \
	$(JAVASCRIPTDIR) \
	$(GOOMDIR)       \
	$(FLASHDIR)

# XXX(godog) after lib/javascript is gone remove this
DIST_SUBDIRS = ccvt \
	slw              \
	sdl_image        \
	sdl_ttf          \
	sdl_gfx          \
	goom             \
	flash
