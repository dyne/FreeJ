/* breaks compilation with swig .31 (due to mess with overloaded function) */
%ignore show_osd();

%include "pydocstrings.i"

/*
should probably be handled in some way..
%typemap(out) Entry *
{
  $1_type pointer = static_cast<Filter*>($1);
  $result = SWIG_NewPointerObj((void *)pointer, SWIGTYPE_p_Filter, 1);
}
*/
#undef freej_entry_typemap_in
%define freej_entry_typemap_in(TypeName)
 %typemap(in) TypeName *
 {
  void *temp_inp;
  int res = SWIG_ConvertPtr($input, &temp_inp,SWIGTYPE_p_ ## TypeName, 0 |  0 );
  if (!SWIG_IsOK(res))
  {
    res = SWIG_ConvertPtr($input, &temp_inp,SWIGTYPE_p_Entry, 0 |  0 );
    if (!SWIG_IsOK(res))
    {
      SWIG_exception_fail(SWIG_ArgError(res), "parameter is not a filter");
    }
    $1 = static_cast< ## TypeName ## *>(reinterpret_cast<Entry*>(temp_inp));
  }
  else
  {
    $1 = reinterpret_cast< ## TypeName ## *>(temp_inp);
  }
 }
%enddef


