
//
//  QTExporter.h
//  freej
//
//  Created by xant on 8/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

@interface QTExporter : NSObject {
    QTMovie *mMovie;
    DataHandler mDataHandlerRef;
    NSDictionary *encodingProperties;
}
- (void)addImage:(NSImage *)image;
- (void)addImages:(NSArray *)imageFilesArray;
@end
