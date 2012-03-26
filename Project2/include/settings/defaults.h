#ifndef DEFAULTS_H_INCLUDED
#define DEFAULTS_H_INCLUDED

#include "iniReader.h"

#define STR(x) #x
#define SETD(sec,op,val) settings.addDefault(STR(sec),STR(op),STR(val))
#define SETDP(sec,op,val) settings->addDefault(STR(sec),STR(op),STR(val))

/** Initialize all the default settings
 *
 *	If a setting doesn't show up and somewhere
 *	its value is extracted it will either cause
 *	an error or return some bogus data. To fix
 *	this, any default settings known at compile
 *	time should be set in this function using
 *	the macros above for simplicity.
 */
void setDefaults(INIReader* settings);


/** @def STR
 *	Just converts macro argument to quoted string
 */

/** @def SETD
 *	Sets the default of section:option to val. This macro
 *	only works if the iniReader is defined in this scope
 *	as "settings".
 */

/** @def SETDP
 *	Same as SETD but settings should be a pointer.
 *
 *	@see SETD
 */
#endif
