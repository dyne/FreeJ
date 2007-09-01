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
MSG_DEF(JSSMSG_FJ_CANT_CREATE,           1, 2, JSEXN_ERR, "can't create {0}: {1}") 
MSG_DEF(JSSMSG_FJ_WICKED,                2, 2, JSEXN_URIERR, "Something wicked happened in '{0}': {1}") 




