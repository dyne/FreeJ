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

#import <CVScreenView.h>
#import <QTExporter.h>
#include <fps.h>

@implementation QTExporter

- (id)initWithScreen:(CVScreenView *)cvscreen
{
    mDataHandlerRef = nil;
    mMovie = nil;
    outputFile = nil;
    screen = cvscreen;
    savedMovieAttributes = [NSDictionary 
        dictionaryWithObjects:
            [NSArray arrayWithObjects:
                [NSNumber numberWithBool:YES],
                [NSNumber numberWithBool:YES],
                [NSNumber numberWithLong:'mpg4'],  // TODO - allow to select the codec
                nil]
        forKeys:
            [NSArray arrayWithObjects:
                QTMovieFlatten, QTMovieExport, QTMovieExportType, nil]];
    
    // when adding images we must provide a dictionary
	// specifying the codec attributes
    encodingProperties = [[NSDictionary dictionaryWithObjectsAndKeys:@"mp4v",
                           QTAddImageCodecType,
                           [NSNumber numberWithLong:codecNormalQuality],
                           QTAddImageCodecQuality,
                           nil] retain];
    
    lock = [[NSRecursiveLock alloc] init];
    memset(&lastTimestamp, 0, sizeof(lastTimestamp));
    lastImage = nil;
    return [super init];
}

- (void)dealloc
{
    [savedMovieAttributes release];
    [encodingProperties release];
    [lock release];
    [super dealloc];
}

- (Movie)quicktimeMovieFromTempFile:(DataHandler *)outDataHandler error:(OSErr *)outErr
{
	*outErr = -1;
	
	// generate a name for our movie file
	NSString *tempName = [NSString stringWithCString:tmpnam(nil) 
                                            encoding:[NSString defaultCStringEncoding]];
	if (!tempName) 
        return nil;
	
	Handle	dataRefH		= nil;
	OSType	dataRefType;
    
	// create a file data reference for our movie
	*outErr = QTNewDataReferenceFromFullPathCFString((CFStringRef)tempName,
                                                     kQTPOSIXPathStyle,
                                                     0,
                                                     &dataRefH,
                                                     &dataRefType);
	if (*outErr != noErr) 
        return nil;
	
	// create a QuickTime movie from our file data reference
	Movie	qtMovie	= nil;
	CreateMovieStorage (dataRefH,
						dataRefType,
						'TVOD',
						smSystemScript,
						newMovieActive, 
						outDataHandler,
						&qtMovie);
	*outErr = GetMoviesError();
	if (*outErr != noErr) goto cantcreatemovstorage;
    
	return qtMovie;
    
    // error handling
cantcreatemovstorage:
	DisposeHandle(dataRefH);
    
	return nil;
}

- (BOOL)pathExists:(NSString *)aFilePath
{
	NSFileManager *defaultMgr = [NSFileManager defaultManager]; 
	
	return [defaultMgr fileExistsAtPath:aFilePath];
}

//
// writeSafelyToURL
//
// Write the document to a movie file
//
//
- (BOOL)writeSafelyToURL:(NSURL *)absoluteURL 
{
    BOOL success = NO;

    if ([self pathExists:[absoluteURL path]] == YES)
    {
        // movie file already exists, so we'll just update
        // the movie resource
        success = [mMovie updateMovieFile];
    }
    else
    {
        success = [mMovie writeToFile:outputFile withAttributes:savedMovieAttributes];
        // movie file does not exist, so we'll flatten our in-memory 
        // movie to the file
        
        // now we can release our old in-memory movie
        [mMovie release];
        mMovie = nil;
        // ...and re-acquire our movie from the new movie file
        mMovie = [QTMovie movieWithFile:[absoluteURL path] error:nil];
        [mMovie retain];
        
        // set the new movie to the view
        //[[mWinController movieView] setMovie:mMovie];
    }
    
    return success;
}

//
// addImage
//
// given an array a CIImage pointer, convert it to NSImage * 
// and add the resulting image to the movie as a new MPEG4 frame
//

- (void)addImage:(CIImage *)image
{
    NSImage *nsImage = [[NSImage alloc] initWithSize:NSMakeSize([image extent].size.width, [image extent].size.height)];
    [nsImage addRepresentation:[NSCIImageRep imageRepWithCIImage:image]];
    // create a QTTime value to be used as a duration when adding 
    // the image to the movie
	//long timeScale      = [[mMovie attributeForKey:@"QTMovieTimeScaleAttribute"] longValue];
    //long long timeValue = timeScale/25;
	QTTime duration;//     = QTMakeTime(timeValue, timeScale);
	// iterate over all the images in the array and add
	// them to our movie one-by-one
    //NSLog(@"%d\n", [images count]);
    CVTimeStamp now;
    memset(&now, 0 , sizeof(now));
    if (CVDisplayLinkGetCurrentTime((CVDisplayLinkRef)[screen getDisplayLink], &now) == kCVReturnSuccess) {
        if (lastImage) {
            int64_t timedelta = lastTimestamp.videoTime?now.videoTime-lastTimestamp.videoTime:now.videoTimeScale/25;            
            duration = QTMakeTime(timedelta, now.videoTimeScale);
            memcpy(&lastTimestamp, &now, sizeof(lastTimestamp));
            
            [QTMovie enterQTKitOnThread];   
            
            // Adds an image for the specified duration to the QTMovie
            [mMovie addImage:lastImage 
                 forDuration:duration
              withAttributes:encodingProperties];
            
            // free up our image object
            if ([self isRunning])
                [self writeSafelyToURL:[NSURL fileURLWithPath:outputFile]];
            [QTMovie exitQTKitOnThread];
            
        }
        if (lastImage)
            [lastImage release];
        lastImage = nsImage;
        
    } else {
        /* TODO - Error Messages */
    }
    
bail:
	return;
  
}

- (BOOL)openOutputMovie
{
    /*  
     NOTES ABOUT CREATING A NEW ("EMPTY") MOVIE AND ADDING IMAGE FRAMES TO IT
     
     In order to compose a new movie from a series of image frames with QTKit
     it is of course necessary to first create an "empty" movie to which these
     frames can be added. Actually, the real requirements (in QuickTime terminology)
     for such an "empty" movie are that it contain a writable data reference. A
     movie with a writable data reference can then accept the addition of image 
     frames via the -addImage method.
     
     Prior to QuickTime 7.2.1, QTKit did not provide a QTMovie method for creating a 
     QTMovie with a writable data reference. In this case, we can use the native 
     QuickTime API CreateMovieStorage() to create a QuickTime movie with a writable 
     data reference (in our example below we use a data reference to a file). We then 
     use the QTKit movieWithQuickTimeMovie: method to instantiate a QTMovie from this 
     native QuickTime movie. 
     
     Finally, images are added to the movie as movie frames using -addImage.
     
     NEW IN QUICKTIME 7.2.1
     
     QuickTime 7.2.1 now provides a new method:
     
     - (id)initToWritableFile:(NSString *)filename error:(NSError **)errorPtr;
     
     to create a QTMovie with a writable data reference. This eliminates the need to
     use the native QuickTime API CreateMovieStorage() as described above.
     
     The code below checks first to see if this new method initToWritableFile: is 
     available, and if so it will use it rather than use the native API.
     */
    //[QTMovie enterQTKitOnThread];
    
    // Check first if the new QuickTime 7.2.1 initToWritableFile: method is available
    if ([[[[QTMovie alloc] init] autorelease] respondsToSelector:@selector(initToWritableFile:error:)] == YES)
    {
        NSError *error;
        NSFileHandle *ofile;
        ofile =  [NSFileHandle fileHandleForUpdatingAtPath:outputFile];
        if (ofile) {
            unsigned long ticks;
            [ofile truncateFileAtOffset:0];
            [ofile synchronizeFile];
            [ofile closeFile];
            // let the os really sync the filesystem before trying to reopen the file
            Delay(5, &ticks);
        }
        // Create a QTMovie with a writable data reference
        mMovie = [[QTMovie alloc] initToWritableFile:outputFile error:&error];
        if (!mMovie) {
            NSLog(@"Can't open output file: %@ : %@", outputFile, [error localizedFailureReason]);
            return NO;
        }
    }
    else    
    {    
        // The QuickTime 7.2.1 initToWritableFile: method is not available, so use the native 
        // QuickTime API CreateMovieStorage() to create a QuickTime movie with a writable 
        // data reference
        
        OSErr err;
        // create a native QuickTime movie
        Movie qtMovie = [self quicktimeMovieFromTempFile:&mDataHandlerRef error:&err];
        if (nil == qtMovie) {
            [lock unlock];
            return NO;
        }
        
        // instantiate a QTMovie from our native QuickTime movie
        mMovie = [QTMovie movieWithQuickTimeMovie:qtMovie disposeWhenDone:YES error:nil];
        if (!mMovie || err) { 
            [lock unlock];
            return NO;
        }
    }
    
    
	// mark the movie as editable
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	// keep it around until we are done with it...
	[mMovie retain];
    //[mMovie setIdling:NO];
    //[mMovie setAttribute:[NSNumber numberWithLong:1000000000] forKey:QTMovieTimeScaleAttribute];
    //[QTMovie exitQTKitOnThread];
    return YES;
}

- (void) exporterThread:(id)arg
{
    NSAutoreleasePool *pool;
    FPS fps;
    fps.init(60); // XXX 
	if (!encodingProperties)
        ;// TODO - Handle error condition
    while ([self isRunning]) {
        pool = [[NSAutoreleasePool alloc] init];
        [self addImage:[screen exportSurface]];
        fps.calc();
        fps.delay();
        [pool release];
    }
}

//
// buildQTKitMovie
//
// Build a QTKit movie from a series of image frames
//
//

- (BOOL)startExport
{
    if (!outputFile)
        outputFile = [[NSString stringWithCString:DEFAULT_OUTPUT_FILE  encoding:NSUTF8StringEncoding] retain];
    if ([self openOutputMovie]) {
        [NSThread detachNewThreadSelector:@selector(exporterThread:) 
                                 toTarget:self withObject:nil];
        return YES;

    }
    return NO;
}

- (BOOL)setOutputFile:(NSString *)path
{
    if (outputFile)
        [outputFile release];
    outputFile = [path retain];
    return YES;
}

- (void)stopExport
{
    [lock lock];
    if (mMovie)
        [mMovie release];
    mMovie = nil;
    if (mDataHandlerRef) {
        CFRelease(mDataHandlerRef);
        mDataHandlerRef = nil;
    }
    [lock unlock];
}

- (BOOL)isRunning
{
    return mMovie?YES:NO;
}

@end
