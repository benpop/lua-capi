#include "lua.h"
#include "lauxlib.h"


#define LIBNAME "capi"


/* XXX: assume lua_Integer max == LONG_MAX */
#ifdef LUA_NUMBER_DOUBLE
#define push_size(L,n)                      \
    if ((n) <= (size_t)LONG_MAX)            \
      lua_pushinteger(L, (lua_Integer)(n)); \
    else                                    \
      lua_pushnumber(L, (lua_Number)(n))
#else
#define push_size(L,n)                      \
    if ((n) <= (size_t)LONG_MAX)            \
      lua_pushinteger(L, (lua_Integer)(n)); \
    else                                    \
      luaL_error(L, "size is too large to represent")
#endif


/*{=================================================================
** main library
**==================================================================*/


static int capi_topointer (lua_State *L) {
  const void *ptr = lua_topointer(L, 1);
  if (ptr != NULL)
    lua_pushfstring(L, "%p", ptr);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_stringtopointer (lua_State *L) {
  const char *str = luaL_checkstring(L, 1);
  lua_pushfstring(L, "%p", str);
  return 1;
}


static int capi_tolightuserdata (lua_State *L) {
  const void *ptr;
  if (lua_islightuserdata(L, 1)) {
    lua_settop(L, 1);
    return 1;
  }
  ptr = lua_topointer(L, 1);
  if (ptr != NULL)
    lua_pushlightuserdata(L, (void *)ptr);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_stringtolightuserdata (lua_State *L) {
  const char *str = luaL_checkstring(L, 1);
  lua_pushlightuserdata(L, (void *)str);
  return 1;
}


static int capi_rawlen (lua_State *L) {
  size_t l = lua_rawlen(L, 1);
  push_size(L, l);
  return 1;
}


static int capi_type (lua_State *L) {
  lua_pushinteger(L, lua_type(L, 1));
  return 1;
}


static int capi_typename (lua_State *L) {
  int tt = luaL_checkint(L, 1);
  lua_pushstring(L, lua_typename(L, tt));
  return 1;
}


static int capi_isfunction (lua_State *L) {
  lua_pushboolean(L, lua_isfunction(L, 1));
  return 1;
}


static int capi_iscfunction (lua_State *L) {
  lua_pushboolean(L, lua_iscfunction(L, 1));
  return 1;
}


static int capi_isluafunction (lua_State *L) {
  lua_pushboolean(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1));
  return 1;
}


static int capi_islightuserdata (lua_State *L) {
  lua_pushboolean(L, lua_islightuserdata(L, 1));
  return 1;
}


static int capi_isheavyuserdata (lua_State *L) {
  lua_pushboolean(L, lua_isuserdata(L, 1) && !lua_islightuserdata(L, 1));
  return 1;
}


static int capi_isuserdata (lua_State *L) {
  lua_pushboolean(L, lua_isuserdata(L, 1));
  return 1;
}


static int capi_isthread (lua_State *L) {
  lua_pushboolean(L, lua_isthread(L, 1));
  return 1;
}


static int capi_istable (lua_State *L) {
  lua_pushboolean(L, lua_istable(L, 1));
  return 1;
}


static int capi_isnil (lua_State *L) {
  lua_pushboolean(L, lua_isnil(L, 1));
  return 1;
}


static int capi_isnumber (lua_State *L) {
  lua_pushboolean(L, lua_isnumber(L, 1));
  return 1;
}


static int capi_isstring (lua_State *L) {
  lua_pushboolean(L, lua_isstring(L, 1));
  return 1;
}


static int capi_tointeger (lua_State *L) {
  lua_Integer n;
  int isnum;
  n = lua_tointegerx(L, 1, &isnum);
  if (isnum)
    lua_pushinteger(L, n);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_tounsigned (lua_State *L) {
  lua_Unsigned n;
  int isnum;
  n = lua_tounsignedx(L, 1, &isnum);
  if (isnum)
    lua_pushunsigned(L, n);
  else
    lua_pushnil(L);
  return 1;
}


static int capi_tolstring (lua_State *L) {
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
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


/*}=================================================================*/


/*{=================================================================
** strict tests and conversions
**==================================================================*/


static int capiS_isstring (lua_State *L) {
  lua_pushboolean(L, lua_type(L, 1) == LUA_TSTRING);
  return 1;
}


static int capiS_isnumber (lua_State *L) {
  lua_pushboolean(L, lua_type(L, 1) == LUA_TNUMBER);
  return 1;
}


static int capiS_tostring (lua_State *L) {
  if (lua_type(L, 1) == LUA_TSTRING)
    lua_settop(L, 1);
  else
    lua_pushnil(L);
  return 1;
}


static int capiS_tolstring (lua_State *L) {
  size_t l;
  const char *s;
  if (lua_type(L, 1) != LUA_TSTRING) {
    lua_pushnil(L);
    return 1;
  }
  s = lua_tolstring(L, 1, &l);
  lua_pushlstring(L, s, l);
  push_size(L, l);
  return 2;
}


static int capiS_tonumber (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER)
    lua_settop(L, 1);
  else
    lua_pushnil(L);
  return 1;
}


static int capiS_tointeger (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushinteger(L, lua_tointeger(L, 1));
  else
    lua_pushnil(L);
  return 1;
}


static int capiS_tounsigned (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushunsigned(L, lua_tounsigned(L, 1));
  else
    lua_pushnil(L);
  return 1;
}


/*}=================================================================*/


/*{=================================================================
** type
**==================================================================*/


/* copied from lua-5.2.2/src/ltm.c:22:1 */
/*
  - changed type 2 (index 3) for LUA_TLIGHTUSERDATA
    from "userdata" to "light userdata"
  - NULL-terminated the list for luaL_checkoption
*/
static const char *const Ttypenames[] = {
  "no value",
  "nil", "boolean", "light userdata", "number",
  "string", "table", "function", "userdata", "thread",
  "proto", "upval", NULL
};


/* XXX: make sure these stay consistent with the above */
#define MIN_TYPETAG (-1)
#define MAX_TYPETAG (10)


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
  {"topointer", capi_topointer},
  {"stringtopointer", capi_stringtopointer},
  {"tolightuserdata", capi_tolightuserdata},
  {"stringtolightuserdata", capi_stringtolightuserdata},
  {"rawlen", capi_rawlen},
  {"type", capi_type},
  {"typename", capi_typename},
  {"isnumber", capi_isnumber},
  {"tointeger", capi_tointeger},
  {"tounsigned", capi_tounsigned},
  {"isstring", capi_isstring},
  {"tolstring", capi_tolstring},
  {"isfunction", capi_isfunction},
  {"iscfunction", capi_iscfunction},
  {"isluafunction", capi_isluafunction},
  {"islightuserdata", capi_islightuserdata},
  {"isheavyuserdata", capi_isheavyuserdata},
  {"isuserdata", capi_isuserdata},
  {"isthread", capi_isthread},
  {"istable", capi_istable},
  {"isnil", capi_isnil},
  {"isboolean", capi_isboolean},
  {"toboolean", capi_toboolean},
  {"createtable", capi_createtable},
  {"concat", capi_concat},
  {NULL, NULL}
};


static const luaL_Reg capi_strict_lib[] = {
  {"isnumber", capiS_isnumber},
  {"isstring", capiS_isstring},
  {"tonumber", capiS_tonumber},
  {"tointeger", capiS_tointeger},
  {"tounsigned", capiS_tounsigned},
  {"tostring", capiS_tostring},
  {"tolstring", capiS_tolstring},
  {NULL, NULL}
};


static const luaL_Reg capi_type_lib[] = {
  {"name", capiT_name},
  {"tag", capiT_tag},
  {"iter", capiT_iter},
  {NULL, NULL}
};


static const luaL_Reg capi_alias_lib[] = {
  {"str2ptr", capi_stringtopointer},
  {"str2lud", capi_stringtolightuserdata},
  {"tolud", capi_tolightuserdata},
  {"islud", capi_islightuserdata},
  {"ishud", capi_isheavyuserdata},
  {"isud", capi_isuserdata},
  {NULL, NULL}
};


int luaopen_capi (lua_State *L) {
  luaL_newlib(L, capi_lib);
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
  require_capi_base(L);
  if (!get_sublib(L, name)) {
    const char *dottedname = lua_pushfstring(L, "%s.%s", LIBNAME, name);
    luaL_requiref(L, dottedname, openf, 0);
    lua_remove(L, -2);  /* remove formatted string */
  }
}

#define require_capi_sublib(L,name) \
  require_capi_sublib_(L, #name, luaopen_capi_##name)


#define sublib_open(name)                                       \
  int luaopen_capi_##name (lua_State *L) {                      \
    require_capi_base(L);                                       \
    luaL_newlib(L, capi_##name##_lib);                          \
    lua_pushvalue(L, -1);  /* copy sublib */                    \
    lua_setfield(L, -3, #name);  /* insert sublib into capi */  \
    return 1;  /* return sublib */                              \
  }

sublib_open(strict)
sublib_open(type)
sublib_open(alias)


int luaopen_capi_all (lua_State *L) {
  int savetop;
  require_capi_base(L);
  savetop = lua_gettop(L);
  require_capi_sublib(L, strict);
  require_capi_sublib(L, type);
  require_capi_sublib(L, alias);
  lua_settop(L, savetop);
  return 1;
}
