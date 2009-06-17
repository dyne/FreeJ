/*  FreeJ - Parameter class implementation
 *  (c) Copyright 2007-2009 Denis Rojo <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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

 $Id$

 */

#include <config.h>
#include <parameter.h>

#include <stdio.h>
#include <stdlib.h>
#include <jutils.h>

Parameter::Parameter(Parameter::Type param_type)
  : Entry() {
  switch(param_type) {
  case Parameter::BOOL:
    value = calloc(1, sizeof(bool));
    break;
  case Parameter::NUMBER:
    value = calloc(1, sizeof(double));
    break;
  case Parameter::COLOR:
    value = calloc(3, sizeof(double));
    break;
  case Parameter::POSITION:
    value = calloc(2, sizeof(double));
    break;
  case Parameter::STRING:
    value = calloc(512, sizeof(char));
    break;
  default:
    error("parameter initialized with unknown type: %u", param_type);
  }

  changed = false;

  type = param_type;

  layer_set_f = NULL;
  layer_get_f = NULL;
  filter_set_f = NULL;
  filter_get_f = NULL;

  set_name("unnamed");
  strcpy(description, " ");
}

Parameter::~Parameter() {
  free(value);
}

bool Parameter::set(void *val) {
  ////////////////////////////////////////
  if(type == Parameter::NUMBER) {

    *(float*)value = *(float*)val;
    
    //////////////////////////////////////
  } else if(type == Parameter::BOOL) {

    *(bool*)value = *(bool*)val;
    
    //    act("filter %s parameter %s set to: %e", name, param->name, (double*)value);
    //////////////////////////////////////
  } else if (type == Parameter::POSITION) {
    
    ((double*)value)[0] = ((double*)val)[0];
    ((double*)value)[1] = ((double*)val)[1];
    
    //////////////////////////////////////
  } else if (type==Parameter::COLOR) {

    ((double*)value)[0] = ((double*)val)[0];
    ((double*)value)[1] = ((double*)val)[1];
    ((double*)value)[2] = ((double*)val)[2];
    
    //////////////////////////////////////
  } else if (type==Parameter::STRING) {
    
    strcpy((char*)value, (char*)val);

  } else {
    error("attempt to set value for a parameter of unknown type: %u", type);
    return false;
  }

  changed = true;
  return true;
}

// TODO VERIFY ALL TYPES
bool Parameter::parse(char *p) {
  // parse the strings into value


    //////////////////////////////////////
  if(type == Parameter::NUMBER) {

    func("parsing number parameter");
    if( sscanf(p, "%le", (double*)value) < 1 ) {
      error("error parsing value [%s] for parameter %s", p, name);
      return false;
    }
    func("parameter %s parsed to %g",p, *(double*)value);

    
    //////////////////////////////////////
  } else if(type == Parameter::BOOL) {

    func("parsing bool parameter");
    char *pp;
    for( pp=p; (*pp!='1') & (*pp!='0') ; pp++) {
      if(pp-p>128) {
	error("error parsing value [%s] for parameter %s", p, name);
	return false;
      }
    }
    if(*pp=='1') *(bool*)value = true;
    if(*pp=='0') *(bool*)value = false;
    func("parameter %s parsed to %s",p, ( *(bool*)value == true ) ? "true" : "false" );
 

    //////////////////////////////////////    
  } else if(type == Parameter::POSITION) {

    double *val;
    
    val = (double*)value;
    if( sscanf(p, "%le %le", &val[0], &val[1]) < 1 ) {
      error("error parsing position [%s] for parameter %s", p, name);
      return false;
    }
    func("parameter %s parsed to %g %g",p, val[0], val[1]);


    //////////////////////////////////////
  } else if(type == Parameter::COLOR) {
    
    double *val;

    val = (double*)value;
    if( sscanf(p, "%le %le %le", &val[0], &val[1], &val[2]) < 1 ) {
      error("error parsing position [%s] for parameter %s", p, name);
      return false;
    }
    func("parameter %s parsed to %le %le %le",p, val[0], val[1], val[2]);


    //////////////////////////////////////
  } else {
    error("attempt to set value for a parameter of unknown type: %u", type);
    return false;
  }

  return true;

}

