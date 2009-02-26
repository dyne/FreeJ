//
//  CIAlphaFade.h
//  freej
//
//  Created by xant on 2/24/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <QuartzCore/QuartzCore.h>

@interface CIAlphaFade: CIFilter
{
    CIImage   *inputImage;
    NSNumber  *outputOpacity;
}
- (id)init;
- (NSDictionary *)customAttributes;
- (CIImage *)outputImage;
+ (void)initialize;
+ (CIFilter *)filterWithName: (NSString *)name;
@end