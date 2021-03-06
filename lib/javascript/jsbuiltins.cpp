/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; -*-
 * vim: set ts=8 sw=4 et tw=99:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla SpiderMonkey JavaScript 1.9 code, released
 * May 28, 2008.
 *
 * The Initial Developer of the Original Code is
 *   Andreas Gal <gal@mozilla.com>
 *
 * Contributor(s):
 *   Brendan Eich <brendan@mozilla.org>
 *   Mike Shaver <shaver@mozilla.org>
 *   David Anderson <danderson@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "jsstddef.h"
#include <math.h>

#include "jsapi.h"
#include "jsarray.h"
#include "jsbool.h"
#include "jscntxt.h"
#include "jsgc.h"
#include "jsiter.h"
#include "jslibmath.h"
#include "jsmath.h"
#include "jsnum.h"
#include "prmjtime.h"
#include "jsdate.h"
#include "jsscope.h"
#include "jsstr.h"
#include "jsbuiltins.h"
#include "jstracer.h"

using namespace avmplus;
using namespace nanojit;

extern jsdouble js_NaN;

JS_FRIEND_API(void)
js_SetTraceableNativeFailed(JSContext *cx)
{
    js_SetBuiltinError(cx);
}

/*
 * NB: bool FASTCALL is not compatible with Nanojit's calling convention usage.
 * Do not use bool FASTCALL, use JSBool only!
 */

jsdouble FASTCALL
js_dmod(jsdouble a, jsdouble b)
{
    if (b == 0.0) {
        jsdpun u;
        u.s.hi = JSDOUBLE_HI32_EXPMASK | JSDOUBLE_HI32_MANTMASK;
        u.s.lo = 0xffffffff;
        return u.d;
    }
    jsdouble r;
#ifdef XP_WIN
    /* Workaround MS fmod bug where 42 % (1/0) => NaN, not 42. */
    if (JSDOUBLE_IS_FINITE(a) && JSDOUBLE_IS_INFINITE(b))
        r = a;
    else
#endif
        r = fmod(a, b);
    return r;
}

int32 FASTCALL
js_imod(int32 a, int32 b)
{
    if (a < 0 || b <= 0)
        return -1;
    int r = a % b;
    return r;
}

/* The following boxing/unboxing primitives we can't emit inline because
   they either interact with the GC and depend on Spidermonkey's 32-bit
   integer representation. */

jsval FASTCALL
js_BoxDouble(JSContext* cx, jsdouble d)
{
    int32 i;
    if (JSDOUBLE_IS_INT(d, i) && INT_FITS_IN_JSVAL(i))
        return INT_TO_JSVAL(i);
    JS_ASSERT(JS_ON_TRACE(cx));
    jsval v; /* not rooted but ok here because we know GC won't run */
    if (!js_NewDoubleInRootedValue(cx, d, &v))
        return JSVAL_ERROR_COOKIE;
    return v;
}

jsval FASTCALL
js_BoxInt32(JSContext* cx, int32 i)
{
    if (JS_LIKELY(INT_FITS_IN_JSVAL(i)))
        return INT_TO_JSVAL(i);
    JS_ASSERT(JS_ON_TRACE(cx));
    jsval v; /* not rooted but ok here because we know GC won't run */
    jsdouble d = (jsdouble)i;
    if (!js_NewDoubleInRootedValue(cx, d, &v))
        return JSVAL_ERROR_COOKIE;
    return v;
} 

jsdouble FASTCALL
js_UnboxDouble(jsval v)
{
    if (JS_LIKELY(JSVAL_IS_INT(v)))
        return (jsdouble)JSVAL_TO_INT(v);
    return *JSVAL_TO_DOUBLE(v);
}

int32 FASTCALL
js_UnboxInt32(jsval v)
{
    if (JS_LIKELY(JSVAL_IS_INT(v)))
        return JSVAL_TO_INT(v);
    return js_DoubleToECMAInt32(*JSVAL_TO_DOUBLE(v));
}

int32 FASTCALL
js_DoubleToInt32(jsdouble d)
{
    return js_DoubleToECMAInt32(d);
}

uint32 FASTCALL
js_DoubleToUint32(jsdouble d)
{
    return js_DoubleToECMAUint32(d);
}

jsdouble FASTCALL
js_StringToNumber(JSContext* cx, JSString* str)
{
    const jschar* bp;
    const jschar* end;
    const jschar* ep;
    jsdouble d;

    JSSTRING_CHARS_AND_END(str, bp, end);
    if ((!js_strtod(cx, bp, end, &ep, &d) ||
         js_SkipWhiteSpace(ep, end) != end) &&
        (!js_strtointeger(cx, bp, end, &ep, 0, &d) ||
         js_SkipWhiteSpace(ep, end) != end)) {
        return js_NaN;
    }
    return d;
}

int32 FASTCALL
js_StringToInt32(JSContext* cx, JSString* str)
{
    const jschar* bp;
    const jschar* end;
    const jschar* ep;
    jsdouble d;

    JSSTRING_CHARS_AND_END(str, bp, end);
    if ((!js_strtod(cx, bp, end, &ep, &d) ||
         js_SkipWhiteSpace(ep, end) != end) &&
        (!js_strtointeger(cx, bp, end, &ep, 0, &d) ||
         js_SkipWhiteSpace(ep, end) != end)) {
        return 0;
    }
    return js_DoubleToECMAInt32(d);
}

SideExit* FASTCALL
js_CallTree(InterpState* state, Fragment* f)
{
    union { NIns *code; GuardRecord* (FASTCALL *func)(InterpState*, Fragment*); } u;

    u.code = f->code();
    JS_ASSERT(u.code);

    GuardRecord* rec;
#if defined(JS_NO_FASTCALL) && defined(NANOJIT_IA32)
    SIMULATE_FASTCALL(rec, state, NULL, u.func);
#else
    rec = u.func(state, NULL);
#endif
    VMSideExit* lr = (VMSideExit*)rec->exit;

    if (lr->exitType == NESTED_EXIT) {
        /* This only occurs once a tree call guard mismatches and we unwind the tree call stack.
           We store the first (innermost) tree call guard in state and we will try to grow
           the outer tree the failing call was in starting at that guard. */
        if (!state->lastTreeCallGuard) {
            state->lastTreeCallGuard = lr;
            FrameInfo** rp = (FrameInfo**)state->rp;
            state->rpAtLastTreeCall = rp + lr->calldepth;
        }
    } else {
        /* If the tree exits on a regular (non-nested) guard, keep updating lastTreeExitGuard
           with that guard. If we mismatch on a tree call guard, this will contain the last
           non-nested guard we encountered, which is the innermost loop or branch guard. */
        state->lastTreeExitGuard = lr;
    }

    return lr;
}

JSBool FASTCALL
js_AddProperty(JSContext* cx, JSObject* obj, JSScopeProperty* sprop)
{
    JS_ASSERT(OBJ_IS_NATIVE(obj));
    JS_ASSERT(SPROP_HAS_STUB_SETTER(sprop));

    JS_LOCK_OBJ(cx, obj);

    JSScope* scope = OBJ_SCOPE(obj);
    uint32 slot;
    if (scope->object == obj) {
        JS_ASSERT(!SCOPE_HAS_PROPERTY(scope, sprop));
    } else {
        scope = js_GetMutableScope(cx, obj);
        if (!scope)
            goto exit_trace;
    }

    slot = sprop->slot;
    if (!scope->table && sprop->parent == scope->lastProp && slot == scope->freeslot) {
        if (slot < STOBJ_NSLOTS(obj) && !OBJ_GET_CLASS(cx, obj)->reserveSlots) {
            JS_ASSERT(JSVAL_IS_VOID(STOBJ_GET_SLOT(obj, scope->freeslot)));
            ++scope->freeslot;
        } else {
            if (!js_AllocSlot(cx, obj, &slot))
                goto exit_trace;

            if (slot != sprop->slot) {
                js_FreeSlot(cx, obj, slot);
                goto exit_trace;
            }
        }

        js_ExtendScopeShape(cx, scope, sprop);
        ++scope->entryCount;
        scope->lastProp = sprop;
    } else {
        JSScopeProperty *sprop2 = js_AddScopeProperty(cx, scope, sprop->id,
                                                      sprop->getter,
                                                      sprop->setter,
                                                      SPROP_INVALID_SLOT,
                                                      sprop->attrs,
                                                      sprop->flags,
                                                      sprop->shortid);
        if (sprop2 != sprop)
            goto exit_trace;
    }

    if (js_IsPropertyCacheDisabled(cx))
        goto exit_trace;

    JS_UNLOCK_SCOPE(cx, scope);
    return JS_TRUE;

  exit_trace:
    JS_UNLOCK_SCOPE(cx, scope);
    return JS_FALSE;
}

static JSBool
HasProperty(JSContext* cx, JSObject* obj, jsid id)
{
    // Check that we know how the lookup op will behave.
    for (JSObject* pobj = obj; pobj; pobj = OBJ_GET_PROTO(cx, pobj)) {
        if (pobj->map->ops->lookupProperty != js_LookupProperty)
            return JSVAL_TO_PSEUDO_BOOLEAN(JSVAL_VOID);
        JSClass* clasp = OBJ_GET_CLASS(cx, pobj);
        if (clasp->resolve != JS_ResolveStub && clasp != &js_StringClass)
            return JSVAL_TO_PSEUDO_BOOLEAN(JSVAL_VOID);
    }

    JSObject* obj2;
    JSProperty* prop;
    if (js_LookupPropertyWithFlags(cx, obj, id, JSRESOLVE_QUALIFIED, &obj2, &prop) < 0)
        return JSVAL_TO_PSEUDO_BOOLEAN(JSVAL_VOID);
    if (prop)
        OBJ_DROP_PROPERTY(cx, obj2, prop);
    return prop != NULL;
}

JSBool FASTCALL
js_HasNamedProperty(JSContext* cx, JSObject* obj, JSString* idstr)
{
    jsid id;
    if (!js_ValueToStringId(cx, STRING_TO_JSVAL(idstr), &id))
        return JSVAL_TO_BOOLEAN(JSVAL_VOID);

    return HasProperty(cx, obj, id);
}

JSBool FASTCALL
js_HasNamedPropertyInt32(JSContext* cx, JSObject* obj, int32 index)
{
    jsid id;
    if (!js_Int32ToId(cx, index, &id))
        return JSVAL_TO_BOOLEAN(JSVAL_VOID);

    return HasProperty(cx, obj, id);
}

jsval FASTCALL
js_CallGetter(JSContext* cx, JSObject* obj, JSScopeProperty* sprop)
{
    JS_ASSERT(!SPROP_HAS_STUB_GETTER(sprop));
    jsval v;
    if (!js_GetSprop(cx, sprop, obj, &v))
        return JSVAL_ERROR_COOKIE;
    return v;
}

JSString* FASTCALL
js_TypeOfObject(JSContext* cx, JSObject* obj)
{
    JSType type = JS_TypeOfValue(cx, OBJECT_TO_JSVAL(obj));
    return ATOM_TO_STRING(cx->runtime->atomState.typeAtoms[type]);
}

JSString* FASTCALL
js_TypeOfBoolean(JSContext* cx, int32 unboxed)
{
    /* Watch out for pseudo-booleans. */
    jsval boxed = PSEUDO_BOOLEAN_TO_JSVAL(unboxed);
    JS_ASSERT(JSVAL_IS_VOID(boxed) || JSVAL_IS_BOOLEAN(boxed));
    JSType type = JS_TypeOfValue(cx, boxed);
    return ATOM_TO_STRING(cx->runtime->atomState.typeAtoms[type]);
}

jsdouble FASTCALL
js_BooleanOrUndefinedToNumber(JSContext* cx, int32 unboxed)
{
    if (unboxed == JSVAL_TO_PSEUDO_BOOLEAN(JSVAL_VOID))
        return js_NaN;
    JS_ASSERT(unboxed == JS_TRUE || unboxed == JS_FALSE);
    return unboxed;
}

JSString* FASTCALL
js_BooleanOrUndefinedToString(JSContext *cx, int32 unboxed)
{
    JS_ASSERT(uint32(unboxed) <= 2);
    return ATOM_TO_STRING(cx->runtime->atomState.booleanAtoms[unboxed]);
}

JSObject* FASTCALL
js_Arguments(JSContext* cx)
{
    return NULL;
}

JSObject* FASTCALL
js_NewNullClosure(JSContext* cx, JSObject* funobj, JSObject* proto, JSObject* parent)
{
    JS_ASSERT(HAS_FUNCTION_CLASS(funobj));
    JS_ASSERT(HAS_FUNCTION_CLASS(proto));
    JS_ASSERT(JS_ON_TRACE(cx));

    JSFunction *fun = (JSFunction*) funobj;
    JS_ASSERT(GET_FUNCTION_PRIVATE(cx, funobj) == fun);

    JSObject* closure = (JSObject*) js_NewGCThing(cx, GCX_OBJECT, sizeof(JSObject));
    if (!closure)
        return NULL;

    js_HoldScope(OBJ_SCOPE(proto));
    closure->map = proto->map;
    closure->classword = jsuword(&js_FunctionClass);
    closure->fslots[JSSLOT_PROTO] = OBJECT_TO_JSVAL(proto);
    closure->fslots[JSSLOT_PARENT] = OBJECT_TO_JSVAL(parent);
    closure->fslots[JSSLOT_PRIVATE] = PRIVATE_TO_JSVAL(fun);
    for (unsigned i = JSSLOT_PRIVATE + 1; i != JS_INITIAL_NSLOTS; ++i)
        closure->fslots[i] = JSVAL_VOID;
    closure->dslots = NULL;
    return closure;
}

#define BUILTIN1 JS_DEFINE_CALLINFO_1
#define BUILTIN2 JS_DEFINE_CALLINFO_2
#define BUILTIN3 JS_DEFINE_CALLINFO_3
#define BUILTIN4 JS_DEFINE_CALLINFO_4
#define BUILTIN5 JS_DEFINE_CALLINFO_5
#include "builtins.tbl"
