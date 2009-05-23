//
//  Freej.m
//  freej
//
//  Created by xant on 2/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//
#import <CFreej.h>
#import <CVLayer.h>
#import <CVF0rLayer.h>
#define DEFAULT_FREEJ_WIDTH 400
#define DEFAULT_FREEJ_HEIGHT 300

@implementation CFreej

- (void) consoleOutput:(id)object
{
    NSAutoreleasePool * p = [[NSAutoreleasePool alloc] init];
	char buf[1024];
	struct timeval timeout = { 1, 0 };
	fd_set rfds;
	
	fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

	buf[1024] = 0;
	for (;;) {
		memset(buf, 0, sizeof(buf));
		if ([outputPanel isEditable])
			[outputPanel setEditable:NO];
		FD_ZERO(&rfds);
		FD_SET(stdout_pipe[0], &rfds);
		FD_SET(stderr_pipe[0], &rfds);
		int maxfd = ((stdout_pipe[0] > stderr_pipe[0])?stdout_pipe[0]:stderr_pipe[0]) +1;
		switch (select(maxfd, &rfds, NULL, NULL, &timeout)) {
		case -1:
		case 0:
			break;
		default:
			if (FD_ISSET(stdout_pipe[0], &rfds)) {
				while (read(stdout_pipe[0], buf, sizeof(buf)-1) > 0) {
					NSString *msg = [[NSString alloc] initWithCString:buf encoding:NSASCIIStringEncoding];
					@synchronized (outputPanel)
					{
						[outputPanel setEditable:YES];
						[outputPanel insertText:msg];
						[outputPanel setEditable:NO];
					}
					[msg release];
				}
			}
			if (FD_ISSET(stderr_pipe[0], &rfds)) {
				while (read(stderr_pipe[0], buf, sizeof(buf)-1) > 0) {
					NSString *msg = [[NSString alloc] initWithCString:buf encoding:NSASCIIStringEncoding];
					@synchronized (outputPanel)
					{
						[outputPanel setEditable:YES];
						[outputPanel insertText:msg];
						[outputPanel setEditable:NO];
					}
					[msg release];
				}
			}
		}
	}
    [p release];
}
 
-(void)awakeFromNib
{
	lock = [[NSRecursiveLock alloc] init];
}

-(id)init
{
	if (self = [super init]) 
	{
		pipe(stdout_pipe);
		pipe(stderr_pipe);
		dup2(stdout_pipe[1], fileno(stdout));
		dup2(stderr_pipe[1], fileno(stderr));
		close(stdout_pipe[1]);
		close(stderr_pipe[1]);
		[NSThread detachNewThreadSelector:@selector(consoleOutput:) 
			toTarget:self withObject:nil];
		return self;
	}
	return nil;
}


- (IBAction)openScript:(id)sender 
{	
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
	freej->open_script((char *)[tvarFilename UTF8String]);
	
} // end openScript

- (Context *)getContext
{
	return freej;
}

- (NSRecursiveLock *)getLock
{
	return lock;
}
- (void)start
{
	if (!freej) {
		freej = new Context();
		
		//const char *filename = [[scriptPath stringValue] UTF8String];
		freej->quit = false;
		assert( freej->init(DEFAULT_FREEJ_WIDTH, DEFAULT_FREEJ_HEIGHT, Context::GL_COCOA, 0) );
		freej->plugger.refresh(freej);
		freej->config_check("keyboard.js");
        Filter *gen = freej->generators.begin();
        while (gen) {
            [generatorsButton addItemWithTitle:[NSString stringWithCString:gen->name]];
            gen = (Filter *)gen->next;
        }
	}	
}

- (IBAction)startGenerator:(id)sender
{
    static GenF0rLayer *tmp = NULL;
    
    if (tmp) {
        delete tmp;
        tmp = NULL;
    }
    tmp = new CVF0rLayer(f0rView, (char *)[[[generatorsButton selectedItem] title] UTF8String], freej);
    if(!tmp) return ;
    
    //tmp->start();

    //  tmp->set_fps(env->fps_speed);
   // freej->add_layer(tmp);
   // tmp->active=true;

    notice("generator %s succesfully created", tmp->name);

}

- (IBAction)reset:(id)sender
{
    freej->reset();
}

@end
