Bridge to Lua's C API.  Allow low-level access to Lua internals.  Includes sub-libraries for strict conversions, Lua types, and shortened aliases.

See the Lua C API for details:  http://www.lua.org/manual/5.2/manual.html#4.8

To require:
	capi = require "capi"
	capi = require "capi.all"
	calias = require "capi.alias"  --  or capi.alias
	cstrict = require "capi.strict"  --  or capi.strict
	ctype = require "capi.type"  --  or capi.type

Example usage:
	capi.topointer(func)  -->  pointer representation as hex string
	capi.tolstring(math.pi)  -->  convert to string with length
	capi.createtable(5, 3)  -->  table with 5 array slots and 3 hash slots
	capi.type.name(2)  -->  what type has the number 2?

WARNING:  LUA IS NOT SUPPOSED TO CRASH.  HOWEVER, THIS LIBRARY GIVES YOU ACCESS TO LUA'S INTERNALS WHICH MAY LEAD TO A CRASH.  THIS LIBRARY IS PROVIDED AS-IS, AS A CONVENIENCE, AND THE AUTHORS ARE NOT RESPONSIBLE FOR ANY DAMAGE OR INCONVENIENCE THAT MAY HAPPEN AS A RESULT OF USING THIS LIBRARY.

Currently only support Lua 5.2.

==benpop==
