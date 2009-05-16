//
//  FrameRate.h
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// GOT From apple example : QTQuartzPlayer

#pragma mark
// Frame rate calculator
@interface FrameRate : NSObject
{
  #define NumStamps 50
  int _count;
  double _frequency;
  uint64_t _stamps[NumStamps + 1];
  NSRecursiveLock *lock;

}
-(void)tick:(uint64_t)timestamp;
-(double)rate;
@end
