Bridge to Lua's C API.  Allow low-level access to Lua internals.  Includes sub-libraries for strict conversions, Lua types, and shortened aliases.

See the Lua C API for details:  http://www.lua.org/manual/5.3/manual.html#4.8

To require:
	capi = require "capi"
	capi = require "capi.all"
	calias = require "capi.alias"  --  or capi.alias
	cstrict = require "capi.strict"  --  or capi.strict
	ctypes = require "capi.types"  --  or capi.types

Example usage:
	capi.address(func)  -->  pointer representation as hex string
	capi.tolstring(math.pi)  -->  convert to string with length
	capi.createtable(5, 3)  -->  table with 5 array slots and 3 hash slots
	capi.types.name(2)  -->  what type has the number 2?

WARNING:  LUA IS NOT SUPPOSED TO CRASH.  HOWEVER, THIS LIBRARY GIVES YOU ACCESS TO LUA'S INTERNALS WHICH MAY LEAD TO A CRASH.  THIS LIBRARY IS PROVIDED AS-IS, AS A CONVENIENCE, AND THE AUTHORS ARE NOT RESPONSIBLE FOR ANY DAMAGE OR INCONVENIENCE THAT MAY HAPPEN AS A RESULT OF USING THIS LIBRARY.

Compiled support for Lua 5.1 (untested), 5.2 (tested), 5.3 (tested).

==benpop==
