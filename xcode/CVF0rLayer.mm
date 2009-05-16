//
//  CVF0rLayer.m
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayer.h"


CVF0rLayer::CVF0rLayer(NSObject *vin)
	:CVLayer(vin)
{
	GenF0rLayer::set_name("F0R");
	generator = NULL;
}

