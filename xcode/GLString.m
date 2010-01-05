/*  FreeJ
 *  (c) Copyright 2009 Andrea Guzzo <xant@dyne.org>
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
// Abstract:	Uses Quartz to draw a string into an OpenGL texture.
//              Imported from apple "CocoaGL" example and extended
//              to fit freej integration. 

#import <GLString.h>

// The following is a NSBezierPath category to allow
// for rounded corners of the border

#pragma mark -
#pragma mark NSBezierPath Category

@implementation NSBezierPath (RoundRect)

+ (NSBezierPath *)bezierPathWithRoundedRect:(NSRect)rect cornerRadius:(float)radius {
    NSBezierPath *result = [NSBezierPath bezierPath];
    [result appendBezierPathWithRoundedRect:rect cornerRadius:radius];
    return result;
}

- (void)appendBezierPathWithRoundedRect:(NSRect)rect cornerRadius:(float)radius {
    if (!NSIsEmptyRect(rect)) {
		if (radius > 0.0) {
			// Clamp radius to be no larger than half the rect's width or height.
			float clampedRadius = MIN(radius, 0.5 * MIN(rect.size.width, rect.size.height));
			
			NSPoint topLeft = NSMakePoint(NSMinX(rect), NSMaxY(rect));
			NSPoint topRight = NSMakePoint(NSMaxX(rect), NSMaxY(rect));
			NSPoint bottomRight = NSMakePoint(NSMaxX(rect), NSMinY(rect));
			
			[self moveToPoint:NSMakePoint(NSMidX(rect), NSMaxY(rect))];
			[self appendBezierPathWithArcFromPoint:topLeft     toPoint:rect.origin radius:clampedRadius];
			[self appendBezierPathWithArcFromPoint:rect.origin toPoint:bottomRight radius:clampedRadius];
			[self appendBezierPathWithArcFromPoint:bottomRight toPoint:topRight    radius:clampedRadius];
			[self appendBezierPathWithArcFromPoint:topRight    toPoint:topLeft     radius:clampedRadius];
			[self closePath];
		} else {
			// When radius == 0.0, this degenerates to the simple case of a plain rectangle.
			[self appendBezierPathWithRect:rect];
		}
    }
}

@end


#pragma mark -
#pragma mark GLString

// GLString follows

@implementation GLString

#pragma mark -
#pragma mark Deallocs

- (void) deleteTexture
{
	if (texName && cgl_ctx) {
		(*cgl_ctx->disp.delete_textures)(cgl_ctx->rend, 1, &texName);
		texName = 0; // ensure it is zeroed for failure cases
		cgl_ctx = 0;
	}
}

- (void) dealloc
{
	[self deleteTexture];
	[textColor release];
	[boxColor release];
	[borderColor release];
	[string release];
	[super dealloc];
}

#pragma mark -
#pragma mark Initializers

// designated initializer
- (id) initWithAttributedString:(NSAttributedString *)attributedString
{
	[super init];
	cgl_ctx = NULL;
	texName = 0;
	texSize.width = 0.0f;
	texSize.height = 0.0f;
    if (string)
        [string release];
	string = [attributedString retain];

	staticFrame = NO;
	antialias = YES;
	marginSize.width = 4.0f; // standard margins
	marginSize.height = 2.0f;
	cRadius = 4.0f;
	requiresUpdate = YES;
	// all other variables 0 or NULL
	return self;
}

- (id) initWithString:(NSString *)aString withFont:font withTextColor:(NSColor *)text BoxColor:(NSColor *)box BorderColor:(NSColor *)border
{
    NSMutableDictionary *attribs = [NSMutableDictionary dictionary];
    [attribs
     setObject:font
     forKey:NSFontAttributeName
     ];
    [attribs
     setObject:text
     forKey:NSForegroundColorAttributeName
     ];
    [attribs
     setObject:box
     forKey:NSBackgroundColorAttributeName
    ];
    // XXX - how to use bordercolor now? 
	return [self initWithAttributedString:[[[NSAttributedString alloc] initWithString:aString attributes:attribs] autorelease]];
}

- (id) initWithString:(NSString *)aString withAttributes:(NSDictionary *)attribs
{
	return [self initWithAttributedString:[[[NSAttributedString alloc] initWithString:aString attributes:attribs] autorelease]];
}

- (void) genImage
{
    if (image)
        [image release];
    if (bitmap)
        [bitmap release];
	
	if ((NO == staticFrame)) { // find frame size if we have not already found it
		frameSize = [string size]; // current string size
		frameSize.width += marginSize.width * 2.0f; // add padding
		frameSize.height += marginSize.height * 2.0f;
	}
	image = [[NSImage alloc] initWithSize:frameSize];
	
	[image lockFocus];
	[[NSGraphicsContext currentContext] setShouldAntialias:antialias];
	if ([boxColor alphaComponent]) { // this should be == 0.0f but need to make sure
		[boxColor set]; 
		NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect(NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height) , 0.5, 0.5)
														cornerRadius:cRadius];
		[path fill];
	}
    
	if ([borderColor alphaComponent]) {
		[borderColor set]; 
		NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect(NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height), 0.5, 0.5) 
														cornerRadius:cRadius];
		[path setLineWidth:1.0f];
		[path stroke];
	}
	
	[textColor set]; 
	[string drawAtPoint:NSMakePoint (marginSize.width, marginSize.height)]; // draw at offset position
	bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height)];
	[image unlockFocus];
}

#pragma mark -
#pragma mark Accessors

- (GLuint) texName
{
	return texName;
}

- (NSSize) texSize
{
	return texSize;
}

#pragma mark Text Color

- (void) setTextColor:(NSColor *)color // set default text color
{
	[color retain];
	[textColor release];
	textColor = color;
	requiresUpdate = YES;
}

- (NSColor *) textColor
{
	return textColor;
}

#pragma mark Box Color

- (void) setBoxColor:(NSColor *)color // set default text color
{
	[color retain];
	[boxColor release];
	boxColor = color;
	requiresUpdate = YES;
}

- (NSColor *) boxColor
{
	return boxColor;
}

#pragma mark Border Color

- (void) setBorderColor:(NSColor *)color // set default text color
{
	[color retain];
	[borderColor release];
	borderColor = color;
	requiresUpdate = YES;
}

- (NSColor *) borderColor
{
	return borderColor;
}

#pragma mark Margin Size

// these will force the texture to be regenerated at the next draw
- (void) setMargins:(NSSize)size // set offset size and size to fit with offset
{
	marginSize = size;
	if (NO == staticFrame) { // ensure dynamic frame sizes will be recalculated
		frameSize.width = 0.0f;
		frameSize.height = 0.0f;
	}
	requiresUpdate = YES;
}

- (NSSize) marginSize
{
	return marginSize;
}

#pragma mark Antialiasing
- (BOOL) antialias
{
	return antialias;
}

- (void) setAntialias:(bool)request
{
	antialias = request;
	requiresUpdate = YES;
}


#pragma mark Frame

- (NSSize) frameSize
{
	if ((NO == staticFrame) && (0.0f == frameSize.width) && (0.0f == frameSize.height)) { // find frame size if we have not already found it
		frameSize = [string size]; // current string size
		frameSize.width += marginSize.width * 2.0f; // add padding
		frameSize.height += marginSize.height * 2.0f;
	}
	return frameSize;
}

- (BOOL) staticFrame
{
	return staticFrame;
}

- (void) useStaticFrame:(NSSize)size // set static frame size and size to frame
{
	frameSize = size;
	staticFrame = YES;
	requiresUpdate = YES;
}

- (void) useDynamicFrame
{
	if (staticFrame) { // set to dynamic frame and set to regen texture
		staticFrame = NO;
		frameSize.width = 0.0f; // ensure frame sizes will be recalculated
		frameSize.height = 0.0f;
		requiresUpdate = YES;
	}
}

#pragma mark String

- (void) setString:(NSAttributedString *)attributedString // set string after initial creation
{
	[attributedString retain];
	[string release];
	string = attributedString;
	if (NO == staticFrame) { // ensure dynamic frame sizes will be recalculated
		frameSize.width = 0.0f;
		frameSize.height = 0.0f;
	}
	requiresUpdate = YES;
}

- (void) setString:(NSString *)aString withAttributes:(NSDictionary *)attribs; // set string after initial creation
{
	[self setString:[[[NSAttributedString alloc] initWithString:aString attributes:attribs] autorelease]];
}


#pragma mark -
#pragma mark Drawing
// generates the texture (requires a current opengl context)
- (CVPixelBufferRef) drawOnBuffer:(CVPixelBufferRef)pixelBuffer
{
    
    [self genImage];
    
    int pxWidth = CVPixelBufferGetWidth(pixelBuffer);
    int pxHeight = CVPixelBufferGetHeight(pixelBuffer);
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    void *rasterData = CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    
    // context to draw in, set to pixel buffer's address
    size_t bitsPerComponent = 8; // *not* CGImageGetBitsPerComponent(image);
    CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGContextRef ctxt = CGBitmapContextCreate(rasterData, pxWidth, pxHeight, bitsPerComponent, bytesPerRow, cs, kCGImageAlphaNoneSkipFirst);
    if(ctxt == NULL){
        NSLog(@"could not create context");
        return NULL;
    }
    
    // draw at the center of the provided pixel buffer
    NSGraphicsContext *nsctxt = [NSGraphicsContext graphicsContextWithGraphicsPort:ctxt flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:nsctxt];
    [image compositeToPoint:NSMakePoint((pxWidth-frameSize.width)/2, (pxHeight-frameSize.height)/2) operation:NSCompositeCopy];
    [NSGraphicsContext restoreGraphicsState];
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    CFRelease(ctxt);
    CFRelease(cs);
    
    return pixelBuffer;
}

@end
