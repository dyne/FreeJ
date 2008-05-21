%module freej

%{
#include "context.h"
#include "console.h"
%}

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
%include "linklist.h"
%include "filter.h"
%include "plugger.h"
%include "context.h"
%include "jsync.h"
%include "layer.h"
%include "jutils.h"

/* Language specific extensions */
#if defined(SWIGPYTHON)
  %include "pypost.i"
#endif
// SWIGPERL5, SWIGRUBY, SWIGJAVA, SWIGLUA...

