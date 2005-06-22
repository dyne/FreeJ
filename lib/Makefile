all:  portaudio-v19/lib/libportaudio.a

# PortAudio
portaudio-v19/lib/libportaudio.a:
	$(MAKE) -C portaudio-v19

# Utility rules
clean:
	-$(MAKE) -C portaudio-v19 clean

distclean:
	-$(MAKE) -C portaudio-v19 distclean

.PHONY: clean distclean
