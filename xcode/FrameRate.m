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

#import <FrameRate.h>

// GOT From apple example : QTQuartzPlayer

#pragma mark FrameRate
@implementation FrameRate
-(id)initWithTimeScale:(int32_t)scale;
{
  _count = 0;
  _rate = 0;
  _scale = scale;
  return [super init];
}

-(void)dealloc
{
    [super dealloc];
}

-(void)tick:(int64_t)timestamp;
{
  int i = 0;
  if (_count > NumStamps) {
      for (i = 0; i < _count; i++) {
        _stamps[i] = _stamps[i+1];
      }
      _count = NumStamps;  
  }
  _stamps[_count++] = timestamp;
}

-(double)rate
{
    if (_count > 1)  { // kepp _rate updated
        _rate = _scale/((_stamps[_count - 1] - _stamps[0])/_count);
    }
    return _rate;
}

@end

