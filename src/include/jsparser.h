/*  FreeJ
 *
 *  Copyright (C) 2004
 *  Silvano Galliani aka kysucix <kysucix@dyne.org>
 *  Denis Rojo aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#ifndef __JSPARSER_H__
#define __JSPARSER_H__

#include <context.h>
#include <config.h>
#ifdef WITH_JAVASCRIPT

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks. */
#define STACK_CHUNK_SIZE    8192

//#include <context.h>
//#include <layer.h>
#include <jsapi.h> // spidermonkey header


class JsParser {
    public:
	JsParser(Context *_env);
	~JsParser();
	int open(const char* script_file);
	int open(JSContext *cx, JSObject *obj, const char* script_file);
	int use(JSContext *cx, JSObject *obj, const char* script_file);
	int parse(const char *command);
	void stop();
    void gc();
	char* readFile(FILE *file,int *len);
	int reset();

	JSBool branch_callback(JSContext* Context, JSScript* Script);

	JSContext *js_context;
	JSObject *global_object;

    private:
	JSRuntime *js_runtime;
	void init();
	void init_class(JSContext *cx, JSObject *obj);
};
#endif

#endif
