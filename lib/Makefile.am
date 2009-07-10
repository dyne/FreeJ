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

if BUILD_FREI0R
FREI0RDIR = frei0r
endif

SUBDIRS = ccvt \
	sdl_image        \
	$(SDLGFXDIR)     \
	sdl_ttf          \
	slw              \
	lo               \
	$(SHOUTDIR)      \
	$(FREI0RDIR)     \
	$(JAVASCRIPTDIR) \
	$(GOOMDIR)       \
	$(CWIIDDIR)      \
	$(FLASHDIR)

# XXX(godog) after lib/javascript is gone remove this
DIST_SUBDIRS = ccvt \
	sdl_image        \
	sdl_gfx          \
	sdl_ttf          \
	slw              \
	lo               \
	shout            \
	frei0r           \
	goom             \
	cwiid            \
	flash
