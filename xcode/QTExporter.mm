//
//  QTExporter.m
//  freej
//
//  Created by xant on 8/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "QTExporter.h"

@implementation QTExporter

//
// quicktimeMovieFromTempFile
//
// Creates a QuickTime movie file from a temporary file
//
//

- (id)init
{
    mDataHandlerRef = nil;
    mMovie = nil;
    outputFile = nil;
    return [super init];
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
                                                     kQTNativeDefaultPathStyle,
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
        NSMutableDictionary *savedMovieAttributes = [NSDictionary 
                                                     dictionaryWithObjects:[NSArray arrayWithObjects:
                                                        [NSNumber numberWithBool:YES],
                                                        [NSNumber numberWithBool:YES],
                                                        [NSNumber numberWithLong:'mpg4'],
                                                        nil]
                                                      forKeys:[NSArray arrayWithObjects:
                                                        QTMovieFlatten, QTMovieExport, QTMovieExportType, nil]];
       
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

- (void)addImage:(CIImage *)image
{
    
    NSImage *nsImage = [[NSImage alloc] initWithSize:NSMakeSize([image extent].size.width, [image extent].size.height)];
    [nsImage addRepresentation:[NSCIImageRep imageRepWithCIImage:image]];
    // create a QTTime value to be used as a duration when adding 
    // the image to the movie
	long long timeValue = 25;
	long timeScale      = 600;
	QTTime duration     = QTMakeTime(timeValue, timeScale);
    
    // Adds an image for the specified duration to the QTMovie
    [mMovie addImage:nsImage 
        forDuration:duration
     withAttributes:encodingProperties];
    
    // free up our image object
    [nsImage release];
    
    [self writeSafelyToURL:[NSURL fileURLWithPath:outputFile]];
    
}

//
// addImages
//
// given an array of image file paths (NSString objects), add each
// image to the movie as a new MPEG4 movie frame
//
// Inputs
//		imageFilesArray - an array of image file paths (NSString objects)
//
// Outputs
//		images specified in imageFilesArray are added to movie
//      as new movie frames
//

- (void)addImages:(NSArray *)imageFilesArray
{
	if (!imageFilesArray)
		goto bail;
    
	// iterate over all the images in the array and add
	// them to our movie one-by-one
	int i;
	for (i = 0; i < [imageFilesArray count]; ++i)
	{
		NSImage *anImage = [imageFilesArray objectAtIndex:i];    
        
        if (anImage)
            [self addImage:anImage];

    }
    
bail:
	return;
}

//
// buildQTKitMovie
//
// Build a QTKit movie from a series of image frames
//
//

- (BOOL)startExport
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
    
    // Check first if the new QuickTime 7.2.1 initToWritableFile: method is available
    if ([[[[QTMovie alloc] init] autorelease] respondsToSelector:@selector(initToWritableFile:error:)] == YES)
    {
        // Create a QTMovie with a writable data reference
        mMovie = [[QTMovie alloc] initToWritableFile:outputFile error:NULL];
    }
    else    
    {    
        // The QuickTime 7.2.1 initToWritableFile: method is not available, so use the native 
        // QuickTime API CreateMovieStorage() to create a QuickTime movie with a writable 
        // data reference
        
        OSErr err;
        // create a native QuickTime movie
        Movie qtMovie = [self quicktimeMovieFromTempFile:&mDataHandlerRef error:&err];
        if (nil == qtMovie) goto bail;
        
        // instantiate a QTMovie from our native QuickTime movie
        mMovie = [QTMovie movieWithQuickTimeMovie:qtMovie disposeWhenDone:YES error:nil];
        if (!mMovie || err) goto bail;
    }
    
    
	// mark the movie as editable
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	
	// keep it around until we are done with it...
	[mMovie retain];
    [mMovie setIdling:NO];
    
    
	// when adding images we must provide a dictionary
	// specifying the codec attributes
	encodingProperties = [[NSDictionary dictionaryWithObjectsAndKeys:@"mp4v",
              QTAddImageCodecType,
              [NSNumber numberWithLong:codecHighQuality],
              QTAddImageCodecQuality,
              nil] retain];
	if (!encodingProperties)
		goto bail;
    
    //if (!outputFile)
      //  outputFile = [[NSString stringWithCString:DEFAULT_OUTPUT_FILE  encoding:NSUTF8StringEncoding] retain];
    
bail:
    
	return YES;
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
    if ([self isRunning]) {
        [outputFile release];
        [mMovie release];
        CFRelease(mDataHandlerRef);
        [encodingProperties release];
    }
}

- (BOOL)isRunning
{
    return mMovie?YES:NO;
}

@end
