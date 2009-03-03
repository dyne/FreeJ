/*
 *  CVLayer.h
 *  freej
 *
 *  Created by xant on 2/23/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */
#ifndef __CVLAYER_H__
#define __CVLAYER_H__


#include <layer.h>
#include <context.h>
#include <QuartzCore/CIFilter.h>
#include <QuartzCore/CVImageBuffer.h>

class CVLayer: public Layer {
	protected:
		int height, width;
		Context *freej;
		void *vbuffer;
		int bufsize;
	public:
		id	input;
		NSString *blendMode;
		enum BlendMode {
			Overlay,
			Saturation,
			Screen,
			SoftLight,
			Color,
			ColorBurn,
			ColorDodgeBlend,
			DarkenBlend,
			DifferenceBlend,
			Exclusion,
			HardLight,
			HueBlend,
			LightenBlend,
			Luminosity,
			Multiply
		};

		CVLayer(NSObject *vin);
		~CVLayer();
		void CVLayer::activate();
		bool open(const char *path);
		bool init(Context *freej);
		bool init(Context *ctx, int w, int h);
		void *feed();
		void close();
		
		bool forward();
		bool backward();
		bool backward_one_keyframe();
		
		bool relative_seek(double increment);
		
		bool set_mark_in();
		bool set_mark_out();
		
		void pause();
		
		void set_blend_mode(BlendMode);
		CIImage *gl_texture();
		
};

#endif
