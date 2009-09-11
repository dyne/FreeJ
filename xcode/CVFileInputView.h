//
//  CVFileInputView.h
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CVFILEINPUTVIEW_H__
#define __CVFILEINPUTVIEW_H__

#import <Cocoa/Cocoa.h>
#import "CVLayerView.h"
#import "CVFileInputController.h"

@interface CVFileInputView : CVLayerView {
}

- (IBAction)setMovieTime:(id)sender;
- (IBAction)openFile:(id)sender;
- (IBAction)togglePlay:(id)sender;

@end

#endif
