/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * freej js error messages
 *
 * The format for each JS error message is:
 *
 * MSG_DEF(<SYMBOLIC_NAME>, <ERROR_NUMBER>, <ARGUMENT_COUNT>, <EXCEPTION_NAME>,
 *         <FORMAT_STRING>)
 *
 * where ;
 * <SYMBOLIC_NAME> is a legal C identifer that will be used in the
 * JS engine source.
 *
 * <ERROR_NUMBER> is an unique integral value identifying this error.
 *
 * <ARGUMENT_COUNT> is an integer literal specifying the total number of
 * replaceable arguments in the following format string.
 *
 * <EXCEPTION_NAME> is an exception index from the enum in jsexn.c;
 * JSEXN_NONE for none.  The given exception index will be raised by the
 * engine when the corresponding error occurs.
 *
 * <FORMAT_STRING> is a string literal, optionally containing sequences
 * {X} where X  is an integer representing the argument number that will
 * be replaced with a string value when the error is reported.
 *
 * e.g.
 *
 * MSG_DEF(JSMSG_NOT_A_SUBSPECIES, 73, JSEXN_NONE, 2,
 *         "{0} is not a member of the {1} family")
 *
 * ***** END LICENSE BLOCK ***** */

/*
	Error messages for freejjs. 
*/

MSG_DEF(JSSMSG_FJ_NOT_AN_ERROR,          0, 0, JSEXN_NONE, "<Error #0 is reserved>")
MSG_DEF(JSSMSG_FJ_CANT_CREATE,           1, 2, JSEXN_REFERENCEERR, "can't create {0}: {1}") 
MSG_DEF(JSSMSG_FJ_WICKED,                2, 2, JSEXN_INTERNALERR, "in function '{0}': '{1}'") 
MSG_DEF(JS_JUNK1,                        3, 0, JSEXN_NONE, "fillup")
MSG_DEF(JS_JUNK2,                        4, 0, JSEXN_NONE, "fillup")
MSG_DEF(JS_JUNK3,                        5, 0, JSEXN_NONE, "fillup")
MSG_DEF(JS_JUNK4,                        6, 0, JSEXN_NONE, "fillup")
MSG_DEF(JS_JUNK5,                        7, 0, JSEXN_NONE, "fillup")
MSG_DEF(JSSMSG_FJ_WRONGTYPE,             8, 3, JSEXN_TYPEERR, "in function '{0}': '{1}' is not a '{2}'")

/* from js.msg ... JSEXN_NONE are not catch()able !

MSG_DEF(JSMSG_NOT_AN_ERROR,             0, 0, JSEXN_NONE, "<Error #0 is reserved>")
MSG_DEF(JSMSG_NOT_DEFINED,              1, 1, JSEXN_REFERENCEERR, "{0} is not defined")
MSG_DEF(JSMSG_NO_REG_EXPS,              2, 1, JSEXN_INTERNALERR, "sorry, regular expression are not supported")
MSG_DEF(JSMSG_MORE_ARGS_NEEDED,         3, 3, JSEXN_NONE, "{0} requires more than {1} argument{2}")
MSG_DEF(JSMSG_BAD_CHAR,                 4, 1, JSEXN_NONE, "invalid format character {0}")
MSG_DEF(JSMSG_BAD_TYPE,                 5, 1, JSEXN_NONE, "unknown type {0}")
MSG_DEF(JSMSG_CANT_LOCK,                6, 0, JSEXN_NONE, "can't lock memory")
MSG_DEF(JSMSG_CANT_UNLOCK,              7, 0, JSEXN_NONE, "can't unlock memory")
MSG_DEF(JSMSG_INCOMPATIBLE_PROTO,       8, 3, JSEXN_TYPEERR, "{0}.prototype.{1} called on incompatible {2}")
MSG_DEF(JSMSG_NO_CONSTRUCTOR,           9, 1, JSEXN_NONE, "{0} has no constructor")
MSG_DEF(JSMSG_CANT_ALIAS,              10, 3, JSEXN_NONE, "can't alias {0} to {1} in class {2}")
MSG_DEF(JSMSG_NOT_SCRIPTED_FUNCTION,   11, 1, JSEXN_TYPEERR, "{0} is not a scripted function")
MSG_DEF(JSMSG_BAD_SORT_ARG,            12, 0, JSEXN_TYPEERR, "invalid Array.prototype.sort argument")
MSG_DEF(JSMSG_BAD_ATOMIC_NUMBER,       13, 1, JSEXN_INTERNALERR, "internal error: no index for atom {0}")
MSG_DEF(JSMSG_TOO_MANY_LITERALS,       14, 0, JSEXN_INTERNALERR, "too many literals")
MSG_DEF(JSMSG_CANT_WATCH,              15, 1, JSEXN_NONE, "can't watch non-native objects of class {0}")
MSG_DEF(JSMSG_STACK_UNDERFLOW,         16, 2, JSEXN_INTERNALERR, "internal error compiling {0}: stack underflow at pc {1}")
MSG_DEF(JSMSG_NEED_DIET,               17, 1, JSEXN_INTERNALERR, "{0} too large")
MSG_DEF(JSMSG_TOO_MANY_LOCAL_ROOTS,    18, 0, JSEXN_ERR, "out of local root space")
MSG_DEF(JSMSG_READ_ONLY,               19, 1, JSEXN_ERR, "{0} is read-only")
MSG_DEF(JSMSG_BAD_FORMAL,              20, 0, JSEXN_SYNTAXERR, "malformed formal parameter")

*/
