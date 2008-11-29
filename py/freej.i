%module freej

%{
#include "context.h"
#include "console.h"
%}

%immutable layers_description;
%immutable Parameter::description;

%ignore Linklist::operator[];

%apply unsigned long { uint16_t };

/* Macros that can be redefined for other languages */
/* freej_entry_typemap_in: to be able to map an Entry* to TypeName* */
#define freej_entry_typemap_in(TypeName)

/* Language specific typemaps */
#if defined(SWIGPYTHON)
  %include "pypre.i"
#endif

/* Entry/Derived typemaps so we can use entries when the children
   are required */
freej_entry_typemap_in(Filter);
freej_entry_typemap_in(Layer);
freej_entry_typemap_in(Controller);
freej_entry_typemap_in(Encoder);

/* for Linklist.search (note normally you want to add
   support for dict like access for specific languages) */
%apply int * OUTPUT { int *idx };

/* This dont compile... */
%ignore Layer::layer_gc;

/* Now the freej headers.. */
%include "freej.h"
%include "linklist.h"
%include "filter.h"
%include "blitter.h"
%include "plugger.h"
%include "context.h"
%include "jsync.h"
%include "layer.h"
%include "jutils.h"

%extend Layer
{
  void add_filter(Filter *filter)
  {
    filter->apply(self);
  }
}


/* Language specific extensions */
#if defined(SWIGPYTHON)
  %include "pypost.i"
#elif defined(SWIGRUBY)
  %include "rbpost.i"
#elif defined(SWIGLUA)
  %include "luapost.i"
#endif
// SWIGPERL5, SWIGRUBY, SWIGJAVA, SWIGLUA...

