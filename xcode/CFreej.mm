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
@synthesize freej;
@synthesize layer;
 
-(id)init
{
	if (self = [super init]) {
	  freej = new Context();
	  assert( freej->init(640, 480, Context::SDL, 0) );
	}
	return self;
}

- (IBAction)run:(id)sender {
	layer = new CVideoGrabber();
	layer->init(freej);
	layer->open(NULL);
	layer->start();
	freej->add_layer(layer);
	layer->active = true;
	while (!freej->quit)
		freej->cafudda(1.0);
}
@end
