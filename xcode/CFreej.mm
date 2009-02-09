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
 
-(id)init
{
	if (self = [super init]) {
	  // Initialization code here
	}
	return self;
}

- (IBAction)run:(id)sender {
	char *argv[2] = { "freej", NULL };
	freej_main(1, argv);
}
@end
