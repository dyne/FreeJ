if BUILD_FLASH
FLASHDIR = flash
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

SUBDIRS = ccvt \
	slw              \
	$(SDLTTFDIR)     \
	$(SDLGFXDIR)     \
	$(JAVASCRIPTDIR) \
	$(FLASHDIR)

## XXX(godog) after lib/javascript is gone remove this
#DIST_SUBDIRS = ccvt \
#	slw              \
#	sdl_image        \
#	sdl_ttf          \
#	sdl_gfx          \
#	flash
#
#EXTRA_DIST = javascript
