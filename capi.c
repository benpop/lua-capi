#include <string.h>

#include "lua.h"
#include "lauxlib.h"


#define LIBNAME "capi"


#ifndef MAX_INTEGER /* { */

/*
** When compiled with -ansi (default), on many systems uint64
** uses "unsigned long long" and thus will raise warnings about
** types not specified in standard C89.
*/

#if defined(LUA_INTSIZE) && LUA_INTSIZE >= 3
typedef lua_Unsigned Uint64;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 Uint64;
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
    || defined(HAVE_STDINT_H)
#include <stdint.h>
typedef uint64_t Uint64;
#else
typedef unsigned long long Uint64;
#endif

#define MAX_INTEGER ((lua_Integer)(~(Uint64)0 >> 1))

#endif /* } */


#define push_size(L,n) do {                 \
    if ((n) <= MAX_INTEGER)                 \
      lua_pushinteger(L, (lua_Integer)(n)); \
    else                                    \
      lua_pushnumber(L, (lua_Number)(n));   \
  } while (0)


#if LUA_VERSION_NUM < 502 /* { */

#define lua_rawlen lua_objlen

static int lua_absindex (lua_State *L, int idx) {
  return idx >= 0 ? idx : (1 + idx + lua_gettop(L));
}

static lua_Number lua_tonumberx (lua_State *L, int idx, int *pisnum) {
  lua_Number n = lua_tonumber(L, idx);
  if (pisnum) *pisnum = (n != 0 || lua_type(L, idx) == LUA_TNUMBER);
  return n;
}

static lua_Integer lua_tointegerx (lua_State *L, int idx, int *pisnum) {
  lua_Integer n = lua_tointeger(L, idx);
  if (pisnum) *pisnum = (n != 0 || lua_type(L, idx) == LUA_TNUMBER);
  return n;
}


/* lua 5.3, work1 src/lauxlib.c:875-912 */

/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
static int luaL_getsubtable (lua_State *L, int idx, const char *fname) {
  lua_getfield(L, idx, fname);
  if (lua_istable(L, -1)) return 1;  /* table already there */
  else {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    return 0;  /* false, because did not find table there */
  }
}


/*
** stripped-down 'require'. Calls 'openf' to open a module,
** registers the result in 'package.loaded' table and, if 'glb'
** is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
static void luaL_requiref (lua_State *L, const char *modname,
                           lua_CFunction openf, int glb) {
  lua_pushcfunction(L, openf);
  lua_pushstring(L, modname);  /* argument to open function */
  lua_call(L, 1, 1);  /* open module */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_pushvalue(L, -2);  /* make copy of module (call result) */
  lua_setfield(L, -2, modname);  /* _LOADED[modname] = module */
  lua_pop(L, 1);  /* remove _LOADED table */
  if (glb) {
    lua_pushvalue(L, -1);  /* copy of 'mod' */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}

#else /* }{ */

#define luaL_register(L,n,f) luaL_newlib(L,f)

#endif /* } */


/*{=================================================================
** main library
**==================================================================*/


static int capi_address (lua_State *L) {
  void *ptr;
  luaL_checkany(L, 1);
  if (lua_type(L, 1) == LUA_TSTRING)
    ptr = (void *)lua_tostring(L, 1);
  else
    ptr = (void *)lua_topointer(L, 1);
  if (ptr != NULL)
    lua_pushfstring(L, "%p", ptr);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_tolightudata (lua_State *L) {
  void *ptr;
  luaL_checkany(L, 1);
  if (lua_islightuserdata(L, 1)) {
    lua_settop(L, 1);
    return 1;
  }
  if (lua_type(L, 1) == LUA_TSTRING)
    ptr = (void *)lua_tostring(L, 1);
  else
    ptr = (void *)lua_topointer(L, 1);
  if (ptr != NULL)
    lua_pushlightuserdata(L, ptr);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_rawlen (lua_State *L) {
  size_t l = (luaL_checkany(L, 1), lua_rawlen(L, 1));
  push_size(L, l);
  return 1;
}


static int capi_isfunction (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isfunction(L, 1));
  return 1;
}


static int capi_iscfunction (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_iscfunction(L, 1));
  return 1;
}


static int capi_isluafunction (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1));
  return 1;
}


static int capi_islightudata (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_islightuserdata(L, 1));
  return 1;
}


static int capi_isfulludata (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isuserdata(L, 1) && !lua_islightuserdata(L, 1));
  return 1;
}


static int capi_isuserdata (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isuserdata(L, 1));
  return 1;
}


static int capi_isthread (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isthread(L, 1));
  return 1;
}


static int capi_istable (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_istable(L, 1));
  return 1;
}


static int capi_isnil (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isnil(L, 1));
  return 1;
}


static int capi_isnumber (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isnumber(L, 1));
  return 1;
}


static int capi_isinteger (lua_State *L) {
#if LUA_VERSION_NUM >= 503
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isinteger(L, 1));
#else
  int isnum;
  lua_Number n = (luaL_checkany(L, 1), lua_tonumberx(L, 1, &isnum));
  lua_pushboolean(L, isnum && n == (lua_Integer)n);
#endif
  return 1;
}


static int capi_isstring (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isstring(L, 1));
  return 1;
}


static int capi_tointeger (lua_State *L) {
  int isnum;
  lua_Integer n = (luaL_checkany(L, 1), lua_tointegerx(L, 1, &isnum));
  if (isnum)
    lua_pushinteger(L, n);
  else
    lua_pushnil(L);
  return 1;
}


#if LUA_VERSION_NUM >= 502
static int capi_tounsigned (lua_State *L) {
  int isnum;
  lua_Unsigned n = (luaL_checkany(L, 1), lua_tounsignedx(L, 1, &isnum));
  if (isnum)
    lua_pushunsigned(L, n);
  else
    lua_pushnil(L);
  return 1;
}
#endif


static int capi_tolstring (lua_State *L) {
  size_t l;
  const char *s;
  luaL_checkany(L, 1);
  s = lua_tolstring(L, 1, &l);
  if (s != NULL) {
    lua_pushlstring(L, s, l);
    push_size(L, l);
    return 2;
  }
  else {
    lua_pushnil(L);
    return 1;
  }
}


static int capi_isboolean (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_isboolean(L, 1));
  return 1;
}


static int capi_toboolean (lua_State *L) {
  lua_pushboolean(L, lua_toboolean(L, 1));
  return 1;
}


static int capi_createtable (lua_State *L) {
  int narr = luaL_checkint(L, 1);
  int nrec = luaL_checkint(L, 2);
  lua_createtable(L, narr, nrec);
  return 1;
}


static int capi_concat (lua_State *L) {
  lua_concat(L, lua_gettop(L));
  return 1;
}


/* copied from lua-5.1.5/src/lbaselib.c:421:1 */
static int capi_newproxy (lua_State *L) {
  lua_settop(L, 1);
  lua_newuserdata(L, 0);  /* create proxy */
  if (lua_toboolean(L, 1) == 0)
    return 1;  /* no metatable */
  else if (lua_isboolean(L, 1)) {
    lua_newtable(L);  /* create a new metatable `m' ... */
    lua_pushvalue(L, -1);  /* ... and mark `m' as a valid metatable */
    lua_pushboolean(L, 1);
    lua_rawset(L, lua_upvalueindex(1));  /* weaktable[m] = true */
  }
  else {
    int validproxy = 0;  /* to check if weaktable[metatable(u)] == true */
    if (lua_getmetatable(L, 1)) {
      lua_rawget(L, lua_upvalueindex(1));
      validproxy = lua_toboolean(L, -1);
      lua_pop(L, 1);  /* remove value */
    }
    luaL_argcheck(L, validproxy, 1, "boolean or proxy expected");
    lua_getmetatable(L, 1);  /* metatable is valid; get it */
  }
  lua_setmetatable(L, 2);
  return 1;
}


/*
** Inspection utilites.
*/


static int capi_string2udata (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  void *ud = lua_newuserdata(L, l);
  memcpy(ud, s, l);
  return 1;
}


static int capi_udata2string (lua_State *L) {
  void *ud = (luaL_checktype(L, 1, LUA_TUSERDATA), lua_touserdata(L, 1));
  size_t l = lua_rawlen(L, 1);
  lua_pushlstring(L, (const char *)ud, l);
  return 1;
}


static int capi_hexstring (lua_State *L) {
  luaL_Buffer b;
  size_t i;
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_settop(L, 1);
  luaL_buffinitsize(L, &b, l * 2);
  for (i = 0; i < l; i++) {
    char sbyte[4];
    sprintf(sbyte, "%02x", (int)(unsigned char)s[i]);
    luaL_addlstring(&b, sbyte, 2);
  }
  luaL_pushresult(&b);
  return 1;
}


static int capi_hexbytes (lua_State *L) {
  luaL_Buffer b;
  int i;
  int n = lua_gettop(L);
  if (n == 0)
    return luaL_error(L, "no bytes given");
  luaL_buffinitsize(L, &b, n * 2);
  for (i = 1; i <= n; i++) {
    char sbyte[4];
    int ibyte = luaL_checkint(L, i);
    if (ibyte < 0 || ibyte > 0xff)
      return luaL_argerror(L, i,
          lua_pushfstring(L, "invalid byte value: %d", ibyte));
    sprintf(sbyte, "%02x", ibyte);
    luaL_addlstring(&b, sbyte, 2);
  }
  luaL_pushresult(&b);
  return 1;
}


static int littleendian (void) {
  long endian_test = 1;  /* most long likely larger than char */
  /* If first byte is 1: little-endian.  Else assume big-endian.  */
  return (int)((char *)&endian_test)[0];
}


static int capi_littleendian (lua_State *L) {
  lua_pushboolean(L, littleendian());
  return 1;
}


static int capi_endianness (lua_State *L) {
  lua_pushstring(L, littleendian() ? "little-endian" : "big-endian");
  return 1;
}


/*}=================================================================*/


/*{=================================================================
** strict tests and conversions
**==================================================================*/


static int capiS_isstring (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_type(L, 1) == LUA_TSTRING);
  return 1;
}


static int capiS_isnumber (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushboolean(L, lua_type(L, 1) == LUA_TNUMBER);
  return 1;
}


static int capiS_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  if (lua_type(L, 1) == LUA_TSTRING)
    lua_settop(L, 1);
  else
    lua_pushnil(L);
  return 1;
}


static int capiS_tolstring (lua_State *L) {
  luaL_checkany(L, 1);
  if (lua_type(L, 1) == LUA_TSTRING) {
    size_t l;
    const char *s = lua_tolstring(L, 1, &l);
    lua_pushlstring(L, s, l);
    push_size(L, l);
    return 2;
  }
  lua_pushnil(L);
  return 1;
}


static int capiS_tonumber (lua_State *L) {
  luaL_checkany(L, 1);
  if (lua_type(L, 1) == LUA_TNUMBER)
    lua_settop(L, 1);
  else
    lua_pushnil(L);
  return 1;
}


#if LUA_VERSION_NUM >= 503
static int capiS_tofloat (lua_State *L) {
  luaL_checkany(L, 1);
  if (lua_type(L, 1) == LUA_TNUMBER && !lua_isinteger(L, 1))
    lua_settop(L, 1);
  else
    lua_pushnil(L);
  return 1;
}
#endif


static int capiS_tointeger (lua_State *L) {
  luaL_checkany(L, 1);
#if LUA_VERSION_NUM >= 503
  if (lua_type(L, 1) == LUA_TNUMBER && lua_isinteger(L, 1))
    lua_settop(L, 1);
#else
  if (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushinteger(L, lua_tointeger(L, 1));
#endif
  else
    lua_pushnil(L);
  return 1;
}


#if LUA_VERSION_NUM >= 502
static int capiS_tounsigned (lua_State *L) {
  luaL_checkany(L, 1);
#if LUA_VERSION_NUM >= 503
  if (lua_type(L, 1) == LUA_TNUMBER && lua_isinteger(L, 1))
#else
  if (lua_type(L, 1) == LUA_TNUMBER)
#endif
    lua_pushunsigned(L, lua_tounsigned(L, 1));
  else
    lua_pushnil(L);
  return 1;
}
#endif


/*}=================================================================*/


/*{=================================================================
** types
**==================================================================*/


/* copied from lua-5.2.2/src/ltm.c:22:1 */
/*
  - change type 2 (index 3) for LUA_TLIGHTUSERDATA
    from "userdata" to "light userdata"
  - change type 7 (index 8) for LUA_TUSERDATA
    from "userdata" to "full userdata"
  - remove non-usual Lua types ("proto" and "upval")
  - NULL-terminate the list for luaL_checkoption
*/
static const char *const Ttypenames[] = {
  "no value",
  "nil", "boolean", "light userdata", "number",
  "string", "table", "function", "full userdata", "thread",
  NULL
};


/* XXX: make sure these stay consistent with the above */
#define MIN_TYPETAG (-1)
#define MAX_TYPETAG (8)


static int capiT_name (lua_State *L) {
  lua_pushstring(L, Ttypenames[lua_type(L, 1)+1]);
  return 1;
}


static int capiT_tag (lua_State *L) {
  int tt = luaL_checkoption(L, 1, NULL, Ttypenames);
  lua_pushinteger(L, tt-1);
  return 1;
}


static int Tnext (lua_State *L) {
  int tt = luaL_checkint(L, 2) + 1;
  lua_pushinteger(L, tt);
  if (MIN_TYPETAG <= tt && tt <= MAX_TYPETAG)
    lua_pushstring(L, Ttypenames[tt+1]);
  else
    lua_pushnil(L);
  return lua_isnil(L, -1) ? 1 : 2;
}


static int capiT_iter (lua_State *L) {
  lua_pushcfunction(L, Tnext);
  lua_pushnil(L);  /* no object needed */
  lua_pushinteger(L, -2);  /* initial index */
  return 3;
}


static int capi_type (lua_State *L) {
  lua_pushinteger(L, lua_type(L, 1));
  return 1;
}


static int capi_typename (lua_State *L) {
  int tt = luaL_checkint(L, 1);
  if (MIN_TYPETAG <= tt && tt <= MAX_TYPETAG) {
    lua_pushstring(L, lua_typename(L, tt));
    return 1;
  }
  else
    return luaL_argerror(L, 1,
        lua_pushfstring(L, "invalid type tag '%d'", tt));
}


/*}=================================================================*/


/*==================================================================
** open libraries
**==================================================================*/


/* copied from lua-5.1.5/src/lbaselib.c:637:3 */
static void add_newproxy (lua_State *L) {
  /* `newproxy' needs a weaktable as upvalue */
  lua_createtable(L, 0, 1);  /* new table `w' */
  lua_pushvalue(L, -1);  /* `w' will be its own metatable */
  lua_setmetatable(L, -2);
  lua_pushliteral(L, "kv");
  lua_setfield(L, -2, "__mode");  /* metatable(w).__mode = "kv" */
  lua_pushcclosure(L, capi_newproxy, 1);
  lua_setfield(L, -2, "newproxy");
}


static const luaL_Reg capi_lib[] = {
  {"address", capi_address},
  {"tolightudata", capi_tolightudata},
  {"rawlen", capi_rawlen},
  {"type", capi_type},
  {"typename", capi_typename},
  {"isnumber", capi_isnumber},
  {"tointeger", capi_tointeger},
#if LUA_VERSION_NUM >= 502
  {"tounsigned", capi_tounsigned},
#endif
  {"isinteger", capi_isinteger},
  {"isstring", capi_isstring},
  {"tolstring", capi_tolstring},
  {"isfunction", capi_isfunction},
  {"iscfunction", capi_iscfunction},
  {"isluafunction", capi_isluafunction},
  {"islightudata", capi_islightudata},
  {"isfulludata", capi_isfulludata},
  {"isuserdata", capi_isuserdata},
  {"isthread", capi_isthread},
  {"istable", capi_istable},
  {"isnil", capi_isnil},
  {"isboolean", capi_isboolean},
  {"toboolean", capi_toboolean},
  {"createtable", capi_createtable},
  {"concat", capi_concat},
  {"string2udata", capi_string2udata},
  {"udata2string", capi_udata2string},
  {"hexstring", capi_hexstring},
  {"hexbytes", capi_hexbytes},
  {"littleendian", capi_littleendian},
  {"endianness", capi_endianness},
  {NULL, NULL}
};


static const luaL_Reg capi_strict_lib[] = {
  {"isnumber", capiS_isnumber},
  {"isstring", capiS_isstring},
  {"tonumber", capiS_tonumber},
#if LUA_VERSION_NUM >= 503
  {"tofloat", capiS_tofloat},
#endif
  {"tointeger", capiS_tointeger},
#if LUA_VERSION_NUM >= 502
  {"tounsigned", capiS_tounsigned},
#endif
  {"tostring", capiS_tostring},
  {"tolstring", capiS_tolstring},
  {NULL, NULL}
};


static const luaL_Reg capi_types_lib[] = {
  {"name", capiT_name},
  {"tag", capiT_tag},
  {"iter", capiT_iter},
  {NULL, NULL}
};


static const luaL_Reg capi_alias_lib[] = {
  {"tolud", capi_tolightudata},
  {"islud", capi_islightudata},
  {"isfud", capi_isfulludata},
  {"isud", capi_isuserdata},
  {NULL, NULL}
};


int luaopen_capi (lua_State *L) {
  luaL_register(L, LIBNAME, capi_lib);
  add_newproxy(L);
  return 1;
}


static void require_capi_base (lua_State *L) {
  /* check in package.loaded */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_getfield(L, -1, LIBNAME);
  if (lua_istable(L, -1)) {
    lua_remove(L, -2);  /* remove _LOADED */
    return;
  }
  lua_pop(L, 2);
  /* check in globals */
  lua_getglobal(L, LIBNAME);
  if (lua_istable(L, -1))
    return;
  lua_pop(L, 1);
  /* require from here */
  luaL_requiref(L, LIBNAME, luaopen_capi, 0);
}


static int get_sublib (lua_State *L, const char *name) {
  lua_getfield(L, -1, name);
  if (lua_istable(L, -1))
    return 1;
  else {
    lua_pop(L, 1);
    return 0;
  }
}


static void require_capi_sublib_ (lua_State *L, const char *name,
                                  lua_CFunction openf) {
  /* call with base lib on stack */
  if (!get_sublib(L, name)) {
    const char *dottedname = lua_pushfstring(L, "%s.%s", LIBNAME, name);
    luaL_requiref(L, dottedname, openf, 0);
    lua_remove(L, -2);  /* remove formatted string */
  }
}

#define require_capi_sublib(L,name) \
  require_capi_sublib_(L, #name, luaopen_capi_##name)


#if LUA_VERSION_NUM >= 502
#define SUBLIB_OPEN(name)                                       \
  int luaopen_capi_##name (lua_State *L) {                      \
    require_capi_base(L);                                       \
    luaL_newlib(L, capi_##name##_lib);                          \
    lua_pushvalue(L, -1);  /* copy sublib */                    \
    lua_setfield(L, -3, #name);  /* insert sublib into capi */  \
    return 1;  /* return sublib */                              \
  }
#else
#define SUBLIB_OPEN(name)                                       \
  int luaopen_capi_##name (lua_State *L) {                      \
    require_capi_base(L);                                       \
    luaL_register(L, LIBNAME "." #name, capi_##name##_lib);     \
    return 1;  /* return sublib */                              \
  }
#endif

SUBLIB_OPEN(strict)
SUBLIB_OPEN(types)
SUBLIB_OPEN(alias)


int luaopen_capi_all (lua_State *L) {
  int savetop;
  require_capi_base(L);
  savetop = lua_gettop(L);
  require_capi_sublib(L, strict);
  require_capi_sublib(L, types);
  require_capi_sublib(L, alias);
  lua_settop(L, savetop);
  return 1;
}
