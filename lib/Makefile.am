if BUILD_FLASH
FLASHDIR = flash
endif

if BUILD_CWIID
CWIIDDIR = cwiid
endif

if BUILD_GOOM
GOOMDIR = goom
endif

if BUILD_JAVASCRIPT
JAVASCRIPTDIR = javascript
endif

if BUILD_SHOUT
SHOUTDIR = shout
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

if BUILD_LIBLO
LIBLODIR = lo
endif

SUBDIRS = ccvt \
	slw              \
	$(LIBLODIR)      \
	$(SDLIMAGEDIR)   \
	$(SDLTTFDIR)     \
	$(SDLGFXDIR)     \
	$(SHOUTDIR)      \
	$(JAVASCRIPTDIR) \
	$(GOOMDIR)       \
	$(CWIIDDIR)      \
	$(FLASHDIR)

# XXX(godog) after lib/javascript is gone remove this
DIST_SUBDIRS = ccvt \
	slw              \
	lo               \
	sdl_image        \
	sdl_ttf          \
	sdl_gfx          \
	shout            \
	goom             \
	cwiid            \
	flash
