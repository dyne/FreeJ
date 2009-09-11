/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

//
// GLString.h
//
// Imported from apple "CocoaGL" example and extended
// to fit freej integration. 

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLContext.h>
#include <QuartzCore/CVPixelBuffer.h>

@interface NSBezierPath (RoundRect)
+ (NSBezierPath *)bezierPathWithRoundedRect:(NSRect)rect cornerRadius:(float)radius;

- (void)appendBezierPathWithRoundedRect:(NSRect)rect cornerRadius:(float)radius;
@end

@interface GLString : NSObject {
	CGLContextObj cgl_ctx; // current context at time of texture creation
	GLuint texName;
	NSSize texSize;
	
	NSAttributedString * string;
	NSColor * textColor; // default is opaque white
	NSColor * boxColor; // default transparent or none
	NSColor * borderColor; // default transparent or none
	BOOL staticFrame; // default in NO
	BOOL antialias;	// default to YES
	NSSize marginSize; // offset or frame size, default is 4 width 2 height
	NSSize frameSize; // offset or frame size, default is 4 width 2 height
	float	cRadius; // Corner radius, if 0 just a rectangle. Defaults to 4.0f
	NSImage * image;
	NSBitmapImageRep * bitmap;
	BOOL requiresUpdate;
}

// this API requires a current rendering context and all operations will be performed in regards to thar context
// the same context should be current for all method calls for a particular object instance

// designated initializer
- (id) initWithAttributedString:(NSAttributedString *)attributedString withTextColor:(NSColor *)color withBoxColor:(NSColor *)color withBorderColor:(NSColor *)color;

- (id) initWithString:(NSString *)aString withAttributes:(NSDictionary *)attribs withTextColor:(NSColor *)color withBoxColor:(NSColor *)color withBorderColor:(NSColor *)color;

// basic methods that pick up defaults
- (id) initWithString:(NSString *)aString withAttributes:(NSDictionary *)attribs;
- (id) initWithAttributedString:(NSAttributedString *)attributedString;

- (void) dealloc;

- (GLuint) texName; // 0 if no texture allocated
- (NSSize) texSize; // actually size of texture generated in texels, (0, 0) if no texture allocated

- (NSColor *) textColor; // get the pre-multiplied default text color (includes alpha) string attributes could override this
- (NSColor *) boxColor; // get the pre-multiplied box color (includes alpha) alpha of 0.0 means no background box
- (NSColor *) borderColor; // get the pre-multiplied border color (includes alpha) alpha of 0.0 means no border
- (BOOL) staticFrame; // returns whether or not a static frame will be used

- (NSSize) frameSize; // returns either dynamc frame (text size + margins) or static frame size (switch with staticFrame)

- (NSSize) marginSize; // current margins for text offset and pads for dynamic frame

- (void) genTexture; // generates the texture without drawing texture to current context
- (void) drawWithBounds:(NSRect)bounds; // will update the texture if required due to change in settings (note context should be setup to be orthographic scaled to per pixel scale)
- (void) drawAtPoint:(NSPoint)point;
- (CVPixelBufferRef) drawOnBuffer:(CVPixelBufferRef)pixelBuffer;

// these will force the texture to be regenerated at the next draw
- (void) setMargins:(NSSize)size; // set offset size and size to fit with offset
- (void) useStaticFrame:(NSSize)size; // set static frame size and size to frame
- (void) useDynamicFrame; // set static frame size and size to frame

- (void) setString:(NSAttributedString *)attributedString; // set string after initial creation
- (void) setString:(NSString *)aString withAttributes:(NSDictionary *)attribs; // set string after initial creation

- (void) setTextColor:(NSColor *)color; // set default text color
- (void) setBoxColor:(NSColor *)color; // set default text color
- (void) setBorderColor:(NSColor *)color; // set default text color

- (BOOL) antialias;
- (void) setAntialias:(bool)request;

@end

