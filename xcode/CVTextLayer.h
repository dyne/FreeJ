//
//  CVTextLayer.h
//  freej
//
//  Created by xant on 6/11/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CVLayer.h>
#import <GLString.h>

@interface CVTextLayerView : CVLayerView {
    IBOutlet NSTextView *textView;
}

- (IBAction)startText:(id)sender;


@end

@interface CVTextLayerController : CVLayerController {
    GLString *theString;
    NSString *text;
    bool needsNewFrame;
}

@property (readwrite) NSString *text;
- (IBAction)startText:(id)sender;
@end
