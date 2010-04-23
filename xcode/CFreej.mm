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

#import <CFreej.h>
#include <CVLayer.h>
#include <CVScreen.h>
#include <jsparser.h>

#define DEFAULT_FREEJ_WIDTH 352
#define DEFAULT_FREEJ_HEIGHT 288


@implementation CFreej


// bridge stdout and stderr with the NSTextView outlet (if any)
- (void) consoleOutput:(id)object
{
    [outputlock lock];
    NSAutoreleasePool * p = [[NSAutoreleasePool alloc] init];
    char buf[1024];
    struct timeval timeout;
    fd_set rfds;
    
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

    buf[1024] = 0;
    for (;;) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
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
                        NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:msg];
                        [[outputPanel textStorage] appendAttributedString:attrString];
                        [outputPanel scrollRangeToVisible:NSMakeRange([[[outputPanel textStorage] characters] count], 0)];
                         [attrString release];
                    }
                    [msg release];
                }
            }
            if (FD_ISSET(stderr_pipe[0], &rfds)) {
                while (read(stderr_pipe[0], buf, sizeof(buf)-1) > 0) {
                    NSString *msg = [[NSString alloc] initWithCString:buf encoding:NSASCIIStringEncoding];
                    @synchronized (outputPanel)
                    {
                        NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:msg];
                        [[outputPanel textStorage] appendAttributedString:attrString];
                        [outputPanel scrollRangeToVisible:NSMakeRange([[[outputPanel textStorage] characters] count], 0)];
                        [attrString release];
                    }
                    [msg release];
                }
            }
        }
    }
    [outputlock unlock];
    [p release];
}
 

-(void)awakeFromNib
{
    lock = [[NSRecursiveLock alloc] init];
    outputlock = [[NSLock alloc] init];
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

- (void)openScriptPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{
     if(returnCode == NSOKButton){
         func("openScript we have an OK button");    
     } else if(returnCode == NSCancelButton) {
         func("openScript we have a Cancel button");
         return;
     } else {
         error("doOpen tvarInt not equal 1 or zero = %3d",returnCode);
         return;
     } // end if     
     NSString * tvarDirectory = [panel directory];
     func("openScript directory = %@",tvarDirectory);

     NSString * tvarFilename = [panel filename];
     func("openScript filename = %@",tvarFilename);

    [scriptPath setStringValue:tvarFilename];
    freej->open_script((char *)[tvarFilename UTF8String]);

}

- (IBAction)openScript:(id)sender 
{    
     NSOpenPanel *tvarNSOpenPanelObj = [NSOpenPanel openPanel];
     NSArray *types = [NSArray arrayWithObject:[NSString stringWithUTF8String:"js"]];
     [tvarNSOpenPanelObj 
        beginSheetForDirectory:nil 
        file:nil
        types:types 
        modalForWindow:[sender window]
        modalDelegate:self 
        didEndSelector:@selector(openScriptPanelDidEnd: returnCode: contextInfo:) 
        contextInfo:nil];    
    [tvarNSOpenPanelObj setCanChooseFiles:YES];
}

- (Context *)getContext
{
    return freej;
}

- (NSRecursiveLock *)getLock
{
    return lock;
}

 - (void)updateLayerList:(id)object
{
    NSAutoreleasePool * p = [[NSAutoreleasePool alloc] init];
    
    if (layerSelect) {
        [layerSelect removeAllItems];
        NSMenu *menu = [layerSelect menu];
        screen->layers.lock();
        Layer *lay = (Layer *)screen->layers.begin ();
        while (lay) {
            if (lay->type !=  Layer::GL_COCOA)
                [menu addItemWithTitle:[NSString stringWithUTF8String:lay->name] action:nil keyEquivalent:@""];
            lay = (Layer *)lay->next;
        }
        screen->layers.unlock();
    }
    [layerSelect synchronizeTitleAndSelectedItem];
    [p release];
}

- (void)start
{
    if (!freej) {
        //freej = new CVContext(self);
        freej = new Context();
        
        // override factory defaults
        Factory<Controller>::set_default_classtype("KeyboardController", "cocoa");
        Factory<ViewPort>::set_default_classtype("Screen", "cocoa");
        
        freej->quit = false;
        NSRect frame = [screenView frame];
        screen = (CVScreen *)Factory<ViewPort>::get_instance("Screen");
        screen->init(frame.size.width, frame.size.height, 32);
        freej->add_screen(screen);
        freej->plugger.refresh(freej);
        //freej->config_check("keyboard.js");
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(updateLayerList:)
             name:NSPopUpButtonWillPopUpNotification
             object:layerSelect];  
    }    
}

- (bool)isVisible:(CVLayer *)layer
{
    bool ret = NO;
    screen->layers.lock();
    Layer *lay = (Layer *)screen->layers.begin ();
    while (lay) {
        // TODO - should check against something more unique than the layer name
        if (strcmp(layer->name, lay->name) == 0) {
            ret = YES;
            break;
        }
        lay =  (Layer *)lay->next;
    }
    screen->layers.unlock();
    return ret;
}

- (IBAction)reset:(id)sender
{
    freej->reset();
    // give the engine some time to stop all layers
    Delay(50, NULL); 

    [screenView reset];
}

@end

/*

CVContext::CVContext(CFreej *cfreej) :
    Context()
{
    _cfreej = cfreej;
}

CVContext::~CVContext()
{
}

void CVContext::add_layer(Layer *lay)
{
    Context::add_layer(lay);
}

bool CVContext::init(int wx, int hx, VideoMode videomode, int audiomode)
{
    bool ret = Context::init(wx, hx, videomode, audiomode);
    return ret;
}
*/

