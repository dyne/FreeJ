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

#import <CIAlphaFade.h>

@implementation CIAlphaFade
static CIKernel *alphaFadeKernel = nil;
 
- (id)init
{
    if(alphaFadeKernel == nil)
    {
        NSBundle    *bundle = [NSBundle bundleForClass: [self class]];
		
        NSString    *code = [NSString stringWithContentsOfFile:
                             [bundle pathForResource: @"CIAlphaFade"
                                            ofType: @"cikernel"]
                              encoding:NSASCIIStringEncoding
                              error:NULL];
        NSArray     *kernels = [CIKernel kernelsWithString: code];
 
        alphaFadeKernel = [[kernels objectAtIndex:0] retain];
    }
    return [super init];
}

- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
 
        [NSDictionary dictionaryWithObjectsAndKeys:
			[NSNumber numberWithDouble:  0.0], kCIAttributeMin,
            [NSNumber numberWithDouble:  1.0], kCIAttributeMax,
            [NSNumber numberWithDouble:  0.0], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:  1.0], kCIAttributeSliderMax,
            [NSNumber numberWithDouble:  0.5], kCIAttributeDefault,
            [NSNumber numberWithDouble:  0.0], kCIAttributeIdentity,
            kCIAttributeTypeScalar,            kCIAttributeType,
			nil], @"outputOpacity",
		nil];
}

- (CIImage *)outputImage
{
    CISampler *src = [CISampler samplerWithImage: inputImage];
 
    return [self apply: alphaFadeKernel, src, outputOpacity, nil];
}

+ (void)initialize
{
    [CIFilter registerFilterName: @"CIAlphaFadeBlendMode"
        constructor: self
        classAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
			@"Fade Alpha", kCIAttributeFilterDisplayName,
			[NSArray arrayWithObjects:
			kCICategoryColorAdjustment, kCICategoryVideo,
			kCICategoryStillImage,kCICategoryInterlaced,
			kCICategoryNonSquarePixels,nil], kCIAttributeFilterCategories,
		nil]
	];
}

+ (CIFilter *)filterWithName: (NSString *)name
{
    CIFilter  *filter;
 
    filter = [[self alloc] init];
    return [filter autorelease];
}
@end
