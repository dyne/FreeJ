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

#import <Cocoa/Cocoa.h>

// GOT From apple example : QTQuartzPlayer

#pragma mark
// Frame rate calculator
#define NumStamps 25
@interface FrameRate : NSObject
{
  int _count;
  double _frequency;
  double rate;
  uint64_t _stamps[NumStamps + 1];
  NSRecursiveLock *lock;

}
-(void)tick:(uint64_t)timestamp;
-(double)rate;
@end
