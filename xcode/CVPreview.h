//
//  CVPreview.h
//  freej
//
//  Created by xant on 5/16/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CFreeJ.h>
#import <CVTexture.h>


@interface CVPreview : NSOpenGLView {
    CIContext              *ciContext;
    bool                   needsReshape;
    NSRecursiveLock        *lock;  
    CVTexture              *texture;
    IBOutlet CFreej        *freej;
}

- (void)renderFrame:(CVTexture *)texture;
- (void)clear;

@end
