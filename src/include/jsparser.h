/*  FreeJ
 *  (c) Copyright 2004 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
 *
 */

#ifndef __JSPARSER_H__
#define __JSPARSER_H__

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks.
 */
#define STACK_CHUNK_SIZE    8192

#include <context.h>
#include <jsapi.h> // spidermonkey header
#include <layer.h>

class JsParser {
    public:
	JsParser(Context *_env);
	~JsParser();
	int open(const char* script_file);
	//	int parse();
    private:
	JSRuntime *js_runtime;
	JSContext *js_context;
	JSObject *global_object;
	JSObject *layer_object;
	void init();

	JSPropertySpec layer_properties[3];

	int parse_count;
	//	JSFunctionSpec shell_functions[3];
};
#endif
