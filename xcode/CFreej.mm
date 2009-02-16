//
//  Freej.m
//  freej
//
//  Created by xant on 2/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//
#import "CFreej.h"

extern int freej_main(int argc, char **argv);
@implementation CFreej
 
-(id)init
{
	if (self = [super init]) 
	{
		freej = new Context();
		freej->quit = true;
	}
	return self;	
}

- (void) runFreej 
{
     
}

- (void) runFreej:(id)object
{
    NSAutoreleasePool * p = [[NSAutoreleasePool alloc] init];
	while (!freej->quit)
		freej->cafudda(1.0);
	freej->rem_layer(captureLayer);
	delete(freej->screen);
	freej->screen = NULL;
    [p release];
}

- (IBAction)run:(id)sender 
{
	if (freej->quit) {
		const char *filename = [[scriptPath stringValue] UTF8String];
		freej->quit = false;
		assert( freej->init(640, 480, Context::SDL, 0) );
		freej->plugger.refresh(freej);
		freej->config_check("keyboard.js");
		captureLayer = new CVideoGrabber(captureDevice);
		captureLayer->init(freej);
		captureLayer->open(NULL);
		captureLayer->start();
		freej->add_layer(captureLayer);
		captureLayer->active = true;
		freej->open_script((char *)filename);

		[NSThread detachNewThreadSelector:@selector(runFreej:) 
			toTarget:self withObject:nil];
	}
}

- (IBAction)openScript:(id)sender; {	
     func("doOpen");	
     NSOpenPanel *tvarNSOpenPanelObj	= [NSOpenPanel openPanel];
     NSInteger tvarNSInteger	= [tvarNSOpenPanelObj runModalForTypes:nil];
     if(tvarNSInteger == NSOKButton){
     	func("openScript we have an OK button");	
     } else if(tvarNSInteger == NSCancelButton) {
     	func("openScript we have a Cancel button");
     	return;
     } else {
     	error("doOpen tvarInt not equal 1 or zero = %3d",tvarNSInteger);
     	return;
     } // end if     

     NSString * tvarDirectory = [tvarNSOpenPanelObj directory];
     func("openScript directory = %@",tvarDirectory);

     NSString * tvarFilename = [tvarNSOpenPanelObj filename];
     func("openScript filename = %@",tvarFilename);

	[scriptPath setStringValue:tvarFilename];
	
} // end openScript

@end
