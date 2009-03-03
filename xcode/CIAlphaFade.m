//
//  CIAlphaFade.m
//  freej
//
//  Created by xant on 2/24/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CIAlphaFade.h"


@implementation CIAlphaFade
static CIKernel *alphaFadeKernel = nil;
 
- (id)init
{
    if(alphaFadeKernel == nil)
    {
        NSBundle    *bundle = [NSBundle bundleForClass: [self class]];
		
        NSString    *code = [NSString stringWithContentsOfFile: [bundle
                                pathForResource: @"CIAlphaFade"
                                ofType: @"cikernel"]];
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
