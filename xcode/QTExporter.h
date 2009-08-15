
//
//  QTExporter.h
//  freej
//
//  Created by xant on 8/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import "CVScreen.h"

#define DEFAULT_OUTPUT_FILE "/tmp/freej_export.mov"

@interface QTExporter : NSObject {
    NSString *outputFile;
    QTMovie *mMovie;
    DataHandler mDataHandlerRef;
    NSDictionary *encodingProperties;
}

- (BOOL)setOutputFile:(NSString *)path;
- (void)addImage:(CIImage *)image;
- (void)addImages:(NSArray *)imageFilesArray;
- (BOOL)startExport;
- (void)stopExport;
- (BOOL)isRunning;
@end
