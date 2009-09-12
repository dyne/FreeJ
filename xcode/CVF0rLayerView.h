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

#ifndef __CVF0RLAYERVIEW_H__
#define __CVF0RLAYERVIEW_H__

#import <CVLayerView.h>
#import <CVF0rLayerController.h>


@interface CVF0rLayerView : CVLayerView
{
    NSImage *icon;
    IBOutlet NSPopUpButton *selectButton;
    IBOutlet NSButton *startButton;
}
- (IBAction)start:(id)sender;
- (IBAction)stop:(id)sender;
@end

#endif

