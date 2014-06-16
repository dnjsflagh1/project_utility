/******************************************************************************/
#include "stdafx.h"
#include "LuaManager.h"
#include "LogSystem.h"
#include "DetectionMgr.h"

// MG_CN_MODIFY
#include "IMGPackExterInterface.h"
/******************************************************************************/

namespace MG
{
    /*
    -----------------------------------------------------------------------------
    class		LuaManager
    -----------------------------------------------------------------------------
    */
	// MG_CN_MODIFY
	IMGExternalPackManager* LuaManager::mPackManager = NULL;

    static Str LuaManager_NullVariableInPool = "null";
    
    //-----------------------------------------------------------------------
    LuaManager::LuaManager()
        :mLuaState(NULL)
        ,mAssertStackCount(0)
    {
        mAssertStackCount = 0;
    }

    //-----------------------------------------------------------------------
    LuaManager::~LuaManager()
    {
        unInitialize();
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::initialize()
    {
		//FUNDETECTION(__MG_FUNC__);
        mLuaState = lua_open(); 
        if ( !mLuaState )
        {
            MG_LOG( out_error, ("%s error: lua_open error\n"), __MG_FUNC__ );
            return false;
        }else
        {
            MG_LOG( out_info, ("%s lua_open \n"), __MG_FUNC__ );
            luaL_openlibs(mLuaState);

            registerMyFileLoader();
        }

        return true;
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::unInitialize()
    {
		//FUNDETECTION(__MG_FUNC__);
        if ( mLuaState != NULL )
        {
            MG_LOG( out_info, ("%s lua_close error\n"), __MG_FUNC__ );
            lua_close(mLuaState);
        }
        return true;
    }

    //-----------------------------------------------------------------------
    void LuaManager::restore()
    {
        if (mLuaState)
            lua_close(mLuaState);
        mLuaState = 0;
        mLuaState = lua_open(); 
        if ( !mLuaState )
            MG_LOG( out_error, ("%s error: lua_restore error\n"), __MG_FUNC__ );
        luaL_openlibs(mLuaState);
    }

    //-----------------------------------------------------------------------
    lua_State* LuaManager::getMainLuaState(void)
    {
        return mLuaState;
    }

    //-----------------------------------------------------------------------
    lua_State* LuaManager::getActiveLuaState(void)
    {
        lua_State* L = mLuaState;
        if (lua_isthread(mLuaState, 1)) {
            L = lua_tothread(mLuaState, 1);
        }
        return L;
    }

    //-----------------------------------------------------------------------
    void LuaManager::clearStack(void)
    {
        lua_settop(mLuaState, 1);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    ** {======================================================
    ** load stream functions
    ** =======================================================
    */

    //-----------------------------------------------------------------------
    typedef struct mg_LoadS {
        const char *s;
        size_t size;
    } LoadS;

    //-----------------------------------------------------------------------
    static int mg_readable_lua_stream(const char *filename)
    {
		// MG_CN_MODIFY
		MGExtractedPackFile packfile;
		LuaManager::getPackManager()->extractPackFile(filename, packfile);
		if (packfile.zmem_file)
			return 1;
		else
			return 0;
    }

    //-----------------------------------------------------------------------
    static mg_LoadS mg_open_lua_stream(const char *filename, const char * mode)
    {
        mg_LoadS ls;
        ls.s = NULL;
        ls.size = 0;
        
        // MG_CN_MODIFY
		char* buf = NULL;
		int size = 0;
		if (LuaManager::getPackManager())
		{
			LuaManager::getPackManager()->extractPackFile(filename, buf, size);
			ls.s = buf;
			ls.size = size;
		}

        return ls;
    }

    //-----------------------------------------------------------------------
    static void mg_close_lua_stream(mg_LoadS ls)
    {
        if (ls.s)
        {

        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    ** {======================================================
    ** main load functions
    ** =======================================================
    */
    //-----------------------------------------------------------------------
    static int mg_luaL_loadfile (lua_State *L, const char *filename) {

        int state = 0;

        FILE *f = fopen(filename, "r");  /* try to open file */
        if (f == NULL) 
        {
            // load from zip
            mg_LoadS ls = mg_open_lua_stream(filename,"r");

            if ( ls.s )
            {
                Str name = Str("@") + Str(filename);
                state =  luaL_loadbuffer(L, ls.s, ls.size, name.c_str());

                mg_close_lua_stream(ls);
            }
        }else
        {
            // load from real file
            fclose(f);
            state = luaL_loadfile(L, filename);
        }
        return state;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    ** {======================================================
    ** load file functions
    ** =======================================================
    */
   
    ////-----------------------------------------------------------------------
    //typedef struct mg_LoadF {
    //    int extraline;
    //    FILE *f;
    //    char buff[LUAL_BUFFERSIZE];
    //} LoadF;

    ////-----------------------------------------------------------------------
    //static int mg_errfile (lua_State *L, const char *what, int fnameindex) {
    //    const char *serr = strerror(errno);
    //    const char *filename = lua_tostring(L, fnameindex) + 1;
    //    lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
    //    lua_remove(L, fnameindex);
    //    return LUA_ERRFILE;
    //}

    ////-----------------------------------------------------------------------
    //static const char *mg_getF (lua_State *L, void *ud, size_t *size) {
    //    mg_LoadF *lf = (mg_LoadF *)ud;
    //    (void)L;
    //    if (lf->extraline) {
    //        lf->extraline = 0;
    //        *size = 1;
    //        return "\n";
    //    }
    //    if (feof(lf->f)) return NULL;
    //    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);
    //    return (*size > 0) ? lf->buff : NULL;
    //}

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    ** {======================================================
    ** 'require' function
    ** =======================================================
    */

    //-----------------------------------------------------------------------
    static int mg_readable (const char *filename) {

        FILE *f = fopen(filename, "r");  /* try to open file */
        if (f == NULL) 
        {
            return mg_readable_lua_stream( filename );
        }else
        {
            fclose(f);
            return 1;
        }
        return 0;
    }

    //-----------------------------------------------------------------------
    static const char *mg_pushnexttemplate (lua_State *L, const char *path) {
        const char *l;
        while (*path == *LUA_PATHSEP) path++;  /* skip separators */
        if (*path == '\0') return NULL;  /* no more templates */
        l = strchr(path, *LUA_PATHSEP);  /* find next separator */
        if (l == NULL) l = path + strlen(path);
        lua_pushlstring(L, path, l - path);  /* template */
        return l;
    }

    //-----------------------------------------------------------------------
    static const char *mg_findfile (lua_State *L, const char *name,
        const char *pname) {
            const char *path;
            name = luaL_gsub(L, name, ".", LUA_DIRSEP);
            lua_getfield(L, LUA_ENVIRONINDEX, pname);
            path = lua_tostring(L, -1);
            if (path == NULL)
                luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
            lua_pushliteral(L, "");  /* error accumulator */
            while ((path = mg_pushnexttemplate(L, path)) != NULL) {
                const char *filename;
                filename = luaL_gsub(L, lua_tostring(L, -1), LUA_PATH_MARK, name);
                lua_remove(L, -2);  /* remove path template */
                if (mg_readable(filename))  /* does file exist and is readable? */
                    return filename;  /* return that file name */
                lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
                lua_remove(L, -2);  /* remove file name */
                lua_concat(L, 2);  /* add entry to possible error message */
            }
            return NULL;  /* not found */
    }

    //-----------------------------------------------------------------------
    static void mg_loaderror (lua_State *L, const char *filename) {
        luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
            lua_tostring(L, 1), filename, lua_tostring(L, -1));
    }

    //-----------------------------------------------------------------------
    int mg_loader_Lua(lua_State *L)
    {
        const char *filename;
        const char *name = luaL_checkstring(L, 1);
        filename = mg_findfile(L, name, "path");
        if (filename == NULL) return 1;  /* library not found in this path */
        if (mg_luaL_loadfile(L, filename) != 0)
            mg_loaderror(L, filename);
        return 1;  /* library loaded successfully */
    }

    //-----------------------------------------------------------------------
    //static const char *mkfuncname (lua_State *L, const char *modname) {
    //    const char *funcname;
    //    const char *mark = strchr(modname, *LUA_IGMARK);
    //    if (mark) modname = mark + 1;
    //    funcname = luaL_gsub(L, modname, ".", LUA_OFSEP);
    //    funcname = lua_pushfstring(L, POF"%s", funcname);
    //    lua_remove(L, -2);  /* remove 'gsub' result */
    //    return funcname;
    //}

    //-----------------------------------------------------------------------
    int mg_loader_C(lua_State *L)
    {
        //const char *funcname;
        //const char *name = luaL_checkstring(L, 1);
        //const char *filename = findfile(L, name, "cpath");
        //if (filename == NULL) return 1;  /* library not found in this path */
        //funcname = mkfuncname(L, name);
        //if (ll_loadfunc(L, filename, funcname) != 0)
        //    loaderror(L, filename);
        return 1;  /* library loaded successfully */
    }

    //-----------------------------------------------------------------------
    int mg_loader_Croot(lua_State *L)
    {
        //const char *funcname;
        //const char *filename;
        //const char *name = luaL_checkstring(L, 1);
        //const char *p = strchr(name, '.');
        //int stat;
        //if (p == NULL) return 0;  /* is root */
        //lua_pushlstring(L, name, p - name);
        //filename = findfile(L, lua_tostring(L, -1), "cpath");
        //if (filename == NULL) return 1;  /* root not found */
        //funcname = mkfuncname(L, name);
        //if ((stat = ll_loadfunc(L, filename, funcname)) != 0) {
        //    if (stat != ERRFUNC) loaderror(L, filename);  /* real error */
        //    lua_pushfstring(L, "\n\tno module " LUA_QS " in file " LUA_QS,
        //        name, filename);
        //    return 1;  /* function not found */
        //}
        return 1;
    }

    //-----------------------------------------------------------------------
    static int mg_loader_preload (lua_State *L) {
        const char *name = luaL_checkstring(L, 1);
        lua_getfield(L, LUA_ENVIRONINDEX, "preload");
        if (!lua_istable(L, -1))
            luaL_error(L, LUA_QL("package.preload") " must be a table");
        lua_getfield(L, -1, name);
        if (lua_isnil(L, -1))  /* not found? */
            lua_pushfstring(L, "\n\tno field package.preload['%s']", name);
        return 1;
    }

    //-----------------------------------------------------------------------
    static const int sentinel_ = 0;
    #define sentinel	((void *)&sentinel_)

    //-----------------------------------------------------------------------
    static int mg_ll_require (lua_State *L) {
        const char *name = luaL_checkstring(L, 1);
        int i;
        lua_settop(L, 1);  /* _LOADED table will be at index 2 */
        lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
        lua_getfield(L, 2, name);
        if (lua_toboolean(L, -1)) {  /* is it there? */
            if (lua_touserdata(L, -1) == sentinel)  /* check loops */
                luaL_error(L, "loop or previous error loading module " LUA_QS, name);
            return 1;  /* package is already loaded */
        }
        /* else must load it; iterate over available loaders */
        lua_getfield(L, LUA_ENVIRONINDEX, "loaders");
        if (!lua_istable(L, -1))
            luaL_error(L, LUA_QL("package.loaders") " must be a table");
        lua_pushliteral(L, "");  /* error message accumulator */
        for (i=1; ; i++) {
            lua_rawgeti(L, -2, i);  /* get a loader */
            if (lua_isnil(L, -1))
                luaL_error(L, "module " LUA_QS " not found:%s",
                name, lua_tostring(L, -2));
            lua_pushstring(L, name);
            lua_call(L, 1, 1);  /* call it */
            if (lua_isfunction(L, -1))  /* did it find module? */
                break;  /* module loaded successfully */
            else if (lua_isstring(L, -1))  /* loader returned error message? */
                lua_concat(L, 2);  /* accumulate it */
            else
                lua_pop(L, 1);
        }
        lua_pushlightuserdata(L, sentinel);
        lua_setfield(L, 2, name);  /* _LOADED[name] = sentinel */
        lua_pushstring(L, name);  /* pass name as argument to module */
        lua_call(L, 1, 1);  /* run loaded module */
        if (!lua_isnil(L, -1))  /* non-nil return? */
            lua_setfield(L, 2, name);  /* _LOADED[name] = returned value */
        lua_getfield(L, 2, name);
        if (lua_touserdata(L, -1) == sentinel) {   /* module did not set a value? */
            lua_pushboolean(L, 1);  /* use true as result */
            lua_pushvalue(L, -1);  /* extra copy to be returned */
            lua_setfield(L, 2, name);  /* _LOADED[name] = true */
        }
        return 1;
    }

    //-----------------------------------------------------------------------
    static const lua_CFunction mg_loaders[] =
        //{mg_loader_preload, mg_loader_Lua, mg_loader_C, mg_loader_Croot, NULL};
    {mg_loader_preload, mg_loader_Lua, NULL};

    static const luaL_Reg mg_ll_funcs[] = {
        {"require", mg_ll_require},
        {NULL, NULL}
    };

    //-----------------------------------------------------------------------
    static int mg_register_my_package (lua_State *L)
    {
        ///////////////////////////////////////////////////////////

        const char *luastr = "assert(true)";
        luaL_loadstring(L, luastr);

        ///////////////////////////////////////////////////////////

        /// 环境表压栈
        lua_getfenv(L, -1);
        /// package[loaders] 压栈
        lua_getfield(L, -1, "package");

        // 复制一份package到LUA_ENVIRONINDEX
        //TODO: 需要查看为什么Lua库和这里的LUA_ENVIRONINDEX指向不一样?
        lua_pushvalue(L, -1);
        lua_replace(L, LUA_ENVIRONINDEX);

        lua_getfield(L, -1, "loaders");

        ///////////////////////////////////////////////////////////

        /// 注册 my_loader_Lua
        for (UInt i=0; mg_loaders[i] != NULL; i++) {
            lua_pushcfunction(L, mg_loaders[i]);
            lua_rawseti(L, -2, i+1);
        }
        //TODO:
        /// 注册 my_loader_C
        /// 注册 my_loader_Croot


        /// 移除c函数
        lua_pop(L, 4);

        ///////////////////////////////////////////////////////////
     
        // require 替换

        lua_pushvalue(L, LUA_GLOBALSINDEX);
        luaL_register(L, NULL, mg_ll_funcs);  /* open lib into global table */
        lua_pop(L, 1);

        ///////////////////////////////////////////////////////////

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    ** {======================================================
    ** register  dofile
    ** =======================================================
    */

  
    //-----------------------------------------------------------------------
    Bool LuaManager::dofile( const char *filename )
    {
        // luaL_dofile

        lua_State* L = getActiveLuaState();

        if ( mg_luaL_loadfile(L, filename) ) 
        {
            return false;
        }

        if ( lua_pcall(L, 0, LUA_MULTRET, 0) ) 
        {
            return false;
        }

        return true;
    }

    //-----------------------------------------------------------------------
    void LuaManager::registerMyFileLoader()
    {
        lua_State* L =  getActiveLuaState();

        startAssertStackBalance();
        {
            lua_pushcfunction(L, mg_register_my_package);
            lua_call(L, 0, 0);
        }
        endAssertStackBalance();
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    //-----------------------------------------------------------------------
    Int LuaManager::getStackElementCount(void)
    {
        lua_State* L =  getActiveLuaState();
        return lua_getgccount(L);
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::isString(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_isstring(L, index) ? 1 : 0;
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::isNumber(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_isnumber(L, index) ? 1 : 0;
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::isBoolean(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_toboolean(L, index) ? 1 : 0;
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::isTable(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_istable(L, index);
    }

    //-----------------------------------------------------------------------
    Str LuaManager::getString(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_tostring(L, index);
    }

    //-----------------------------------------------------------------------
    Int  LuaManager::getNumber(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return (Int)lua_tonumber(L, index);
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::getBoolean(Int index)
    {
        lua_State* L =  getActiveLuaState();
        return lua_toboolean(L, index) ? 1 : 0;
    }

    //-----------------------------------------------------------------------
    void LuaManager::setString(const Str& value, Int index)
    {
        lua_State* L =  getActiveLuaState();
        lua_pushstring(L, value.c_str());
        lua_insert(L, index);
    }

    //-----------------------------------------------------------------------
    void LuaManager::setNumber(Int value, Int index)
    {
        lua_State* L =  getActiveLuaState();
        lua_pushnumber(L, value);
        lua_insert(L, index);
    }

    //-----------------------------------------------------------------------
    void LuaManager::removeStackElement(Int index)
    {
        lua_State* L =  getActiveLuaState();
        lua_remove(L, index);
    }

    //-----------------------------------------------------------------------
    void LuaManager::printStack(lua_State *L)
    {
#ifdef _DEBUG

        //UInt stackCount     = lua_gettop( L );

        //MG_LOG( out_sys, ("print stack start \n") );

        //for( UInt i=stackCount; i>0; i-- )
        //{
        //    Str valueStr;

        //    LuaManager::convertLuaStackValueToString(L,i,valueStr);

        //    MG_LOG( out_info, ("Index = %d; Value = %s \n"), i, valueStr.c_str() );
        //}
        //MG_LOG( out_sys, ("print stack end \n") );

#endif
    }
    
    //-----------------------------------------------------------------------
    void LuaManager::getStringListFromTable( std::vector<Str>& slt, Int index )
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: para not a valid table \n"), __MG_FUNC__ );
        }
        Str str;
        lua_pushnil( L );  
        if (index < 0)
            index -= 1; 
        while (lua_next( L, index) != 0) {
            str = luaL_checkstring( L, -1 );
            slt.push_back( str );
            lua_pop( L, 1 );
        }
    }

    //-----------------------------------------------------------------------
    void LuaManager::getVector3ListFromTable( std::vector<Vec3>& poIntList, Int index )
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: getVector3ListFromTable \n"), __MG_FUNC__ );
        }

        Vec3 pos;
        lua_pushnil( L );  
        if (index < 0)
            index -= 1; 
        while (lua_next( L, index) != 0) {
            pos = getVector3FromTable( -1 );
            poIntList.push_back( pos );
            lua_pop( L, 1 );
        }
    }

    //-----------------------------------------------------------------------
    Vec3 LuaManager::getVector3FromTable(Int index)
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: getVector3FromTable \n"), __MG_FUNC__ );
        }

        Flt x = getNumberField("x", index, 1);
        Flt y = getNumberField("y", index, 2);
        Flt z = getNumberField("z", index, 3);

        return Vec3(x,y,z);
    }

    //-----------------------------------------------------------------------
    Vec2 LuaManager::getVector2FromTable(Int index)
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: getVector2FromTable \n"), __MG_FUNC__ );
        }

        Flt x = getNumberField("x", index, 1);
        Flt y = getNumberField("y", index, 2);

        return Vec2(x,y);
    }

    //-----------------------------------------------------------------------
    Vec4 LuaManager::getQuaternionFromTable(Int index)
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: getQuaternionFromTable \n"), __MG_FUNC__ );
        }

        Flt w = getNumberField("w", index, 1);
        Flt x = getNumberField("x", index, 2);
        Flt y = getNumberField("y", index, 3);
        Flt z = getNumberField("z", index, 4);

        return Vec4(w,x,y,z);
    }

    //-----------------------------------------------------------------------
    Color LuaManager::getColourValueFromTable(Int index)
    {
        lua_State* L =  getActiveLuaState();

        if (!lua_istable(L, index))
        {
            MG_LOG( out_error, ("%s error: getColourValueFromTable \n"), __MG_FUNC__ );
        }

        Flt r = getNumberField("r", index, 1);
        Flt g = getNumberField("g", index, 2);
        Flt b = getNumberField("b", index, 3);
        Flt a = getNumberField("a", index, 4);

        return Color(r,g,b,a);
    }

    //-----------------------------------------------------------------------
    Flt LuaManager::getNumberField (const Str& key, Int tableindex, Int defaultindex)
    {
        lua_State* L =  getActiveLuaState();

        Flt result;
        lua_pushstring(L, key.c_str());

        if (tableindex < 0)  tableindex -= 1; 
        lua_gettable(L, tableindex);  /* get table[key] */
        if (!lua_isnumber(L, -1))
        {
            lua_pop(L,1);
            if (tableindex < 0)  tableindex += 1;
            lua_pushnumber(L, defaultindex);
            if (tableindex < 0) tableindex -= 1; 
            lua_gettable(L, tableindex);  /* get table[defaultindex] */

            if (!lua_isnumber(L, -1))
            {
                MG_LOG( out_error, ("%s error: getNumberField \n"), __MG_FUNC__ );
            }
        }

        result = (Flt)lua_tonumber(L, -1);
        lua_pop(L, 1);  /* remove table[key] */
        return result;
    }

	//-----------------------------------------------------------------------
	Flt LuaManager::getNumberField (const Str& key, Int tableindex,Bool& isError)
	{
		lua_State* L =  getActiveLuaState();

		Flt result;
		lua_pushstring(L, key.c_str());

		if (tableindex < 0)  tableindex -= 1; 
		lua_gettable(L, tableindex);  /* get table[key] */
		if (!lua_isnumber(L, -1))
		{
			isError = true;

		#ifndef _DEBUG
			return 0;
		#endif
		}

		result = (Flt)lua_tonumber(L, -1);
		lua_pop(L, 1);  /* remove table[key] */
		isError = false;
		return result;
	}
    //-----------------------------------------------------------------------
    Str LuaManager::getStringField (const Str& key, Int tableindex, Int defaultindex)
    {
        lua_State* L =  getActiveLuaState();

        Str result;
        lua_pushstring(L, key.c_str());

        if (tableindex < 0)  tableindex -= 1; 

        lua_gettable(L, tableindex);  /* get table[key] */
        if (!lua_isstring(L, -1))
        {
            lua_pop(L,1);
            if (tableindex < 0)  tableindex += 1;
            lua_pushnumber(L, defaultindex);
            if (tableindex < 0) tableindex -= 1; 
            lua_gettable(L, tableindex);  /* get table[defaultindex] */

            if (!lua_isstring(L, -1))
            {
                MG_LOG( out_error, ("%s error: getStringField \n"), __MG_FUNC__ );
            }
        }

        result = lua_tostring(L, -1);
        lua_pop(L, 1);  /* remove table[key] */
        return result;
    }

	//-----------------------------------------------------------------------
	Str LuaManager::getStringField (const Str& key, Int tableindex)
	{
		lua_State* L =  getActiveLuaState();

		Str result;
		lua_pushstring(L, key.c_str());

		if (tableindex < 0)  tableindex -= 1; 

		lua_gettable(L, tableindex);  /* get table[key] */
		if (!lua_isstring(L, -1))
		{
		#ifndef _DEBUG
			return "ERROR";	
		#endif
		}

		result = lua_tostring(L, -1);
		lua_pop(L, 1);  /* remove table[key] */
		return result;
	}

    //-----------------------------------------------------------------------
    void LuaManager::createStringListTable(const std::vector<Str>& slt )
    {
        lua_State* L =  getActiveLuaState();

        lua_newtable(L);	
        size_t n = 1;
        for( std::vector<Str>::const_iterator iter = slt.end(); n <= slt.size(); ++n)
        {
            --iter;
            setTableFieldByValue(n, *iter, -1);	
        }	
    }

  
    //-----------------------------------------------------------------------
    void LuaManager::createVector3ListTable(const std::vector<Vec3>& poIntList)
    {
        lua_State* L =  getActiveLuaState();

        lua_newtable(L);	

        //startAssertStackBalance();

        Vec3 pos;
        size_t n = 1;
        for( std::vector<Vec3>::const_iterator iter = poIntList.begin(); iter != poIntList.end(); ++iter, ++n)
        {
            pos = (*iter);
            createVector3Table( pos );

            setTableFieldByIndex( n, -1, -2);
            lua_pop(L, 1);
        }

        //endAssertStackBalance();
    }

    //-----------------------------------------------------------------------
    void LuaManager::createVector3Table(const Vec3& vec)
    {
        lua_State* L =  getActiveLuaState();

        lua_newtable(L);			
        setTableFieldByValue("x", vec.x, -1);		
        setTableFieldByValue("y", vec.y, -1);	
        setTableFieldByValue("z", vec.z, -1);		
    }

    //-----------------------------------------------------------------------
    void LuaManager::createQuaternionTable(const Vec4& quaternion)
    {
        lua_State* L =  getActiveLuaState();

        lua_newtable(L);
        setTableFieldByValue("w", quaternion.w, -1);
        setTableFieldByValue("x", quaternion.x, -1);		
        setTableFieldByValue("y", quaternion.y, -1);	
        setTableFieldByValue("z", quaternion.z, -1);	
    }

    //-----------------------------------------------------------------------
    void LuaManager::createColourValueTable(const Color& colourValue)
    {
        lua_State* L =  getActiveLuaState();

        lua_newtable(L);
        setTableFieldByValue("r", colourValue.r, -1);
        setTableFieldByValue("g", colourValue.g, -1);		
        setTableFieldByValue("b", colourValue.b, -1);	
        setTableFieldByValue("a", colourValue.a, -1);	
    }

    //-----------------------------------------------------------------------
    void LuaManager::setTableFieldByValue(const Str& key, Flt value, Int tableindex)
    {
        lua_State* L =  getActiveLuaState();

        lua_pushstring(L, key.c_str());
        lua_pushnumber(L, (Dbl)value);
        if (tableindex < 0)
            tableindex -= 2; 
        lua_settable(L, tableindex);
    }
    //-----------------------------------------------------------------------
    void LuaManager::setTableFieldByValue(const Str& key, Str value, Int tableindex)
    {
        lua_State* L =  getActiveLuaState();

        lua_pushstring(L, key.c_str());
        lua_pushstring(L, value.c_str());
        if (tableindex < 0)
            tableindex -= 2; 
        lua_settable(L, tableindex);
    }

    //-----------------------------------------------------------------------
    void LuaManager::setTableFieldByValue(size_t key, Str value, Int tableindex)
    {
        lua_State* L =  getActiveLuaState();

        lua_pushstring(L, value.c_str());
        if (tableindex < 0)
            tableindex -= 1; 
        lua_rawseti( L, tableindex, Int(key) );
    }

	//-----------------------------------------------------------------------
	void LuaManager::setTableFieldByValue(size_t key, Flt value, Int tableindex)
	{
		lua_State* L =  getActiveLuaState();

		lua_pushnumber(L, (Dbl)value);
		if (tableindex < 0)
			tableindex -= 1; 
		lua_rawseti( L, tableindex, Int(key) );
	}

    //-----------------------------------------------------------------------
    void LuaManager::setTableFieldByIndex(const Str& key, Int valuetableindex, Int tableindex)
    {
        lua_State* L =  getActiveLuaState();

        lua_pushstring(L, key.c_str());
        if ( valuetableindex < 0 )
            valuetableindex -= 1;
        lua_pushvalue(L, valuetableindex);
        if (tableindex < 0)
            tableindex -= 2; 
        lua_settable(L, tableindex);
    }

    //-----------------------------------------------------------------------
    void LuaManager::setTableFieldByIndex(size_t key, Int valuetableindex, Int tableindex)
    {
        lua_State* L =  getActiveLuaState();

        lua_pushvalue(L, valuetableindex);
        if (tableindex < 0)
            tableindex -= 1; 
        lua_rawseti( L, tableindex, Int(key) );
    }

    //-----------------------------------------------------------------------
    Bool LuaManager::luaPcall( Int nargs, Int nresults )
    {
        lua_State* L =  getMainLuaState();

        if (lua_pcall( L, nargs, nresults, 0 ) != 0)
        {
            Str errorhInt = lua_tostring( L, -1 );		
            MG_LOG( out_error, ("%s <error: luaPcall> \n"), errorhInt.c_str() );
            return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------
    void LuaManager::doStringChunk( const Str& chunk )
    {
        lua_State* L =  getActiveLuaState();

        Int	error = luaL_loadbuffer( L, chunk.c_str(), chunk.length(), "LuaManager::doStringChunk" ) || 
            lua_pcall( L, 0, 0, 0 );
        if (error)
        {
            Str errorhInt = lua_tostring( L, -1 );
            errorhInt = " ! ERROR: " + errorhInt;
            MG_LOG( out_error, ("%s\n"), errorhInt );
        }
    }

    //-----------------------------------------------------------------------
    void LuaManager::convertLuaStackValueToString( lua_State* L, Int stackindex, Str& convertstring )
    {
        convertstring = LuaManager_NullVariableInPool;
        std::stringstream stream;
        Flt number;
        Int t = (Int)lua_type( L, stackindex );
        //String strtype = lua_typename( L, i );

        switch (t) {
            case LUA_TSTRING:  /* strings */
                convertstring = lua_tostring(L, stackindex);
                break;
            
            case LUA_TBOOLEAN:  /* Booleans */
                lua_toboolean(L, stackindex) ? convertstring = "true" : convertstring = "false";
                break;

            case LUA_TNUMBER:  /* numbers */
                number = (Flt)lua_tonumber(L, stackindex);
                stream << number;
                stream >> convertstring;
                break;

            case LUA_TTABLE:  /* table */
                Int point;
                point = (Int)lua_topointer(L, stackindex);
                stream << point;
                stream >> convertstring;
                break;
                
            default:  /* other values */
                break;
        }
    }

    //-----------------------------------------------------------------------
    void LuaManager::convertLuaStackValueToPool( Int stackindex )
    {
        lua_State* L =  getActiveLuaState();

        Str value = LuaManager_NullVariableInPool;
        Str mark  = LuaManager_NullVariableInPool;

        if ( lua_type( L, stackindex ) == LUA_TTABLE )
        {
            lua_pushnil( L );  
            if (stackindex < 0)
                stackindex -= 1; 
            while (lua_next( L, stackindex) != 0) {
                if ( lua_type( L, -1 ) == LUA_TTABLE )
                {
                    convertLuaStackValueToPool( -1 );
                }else
                {
                    convertLuaStackValueToString( L, -1, value );
                }
                convertLuaStackValueToString( L, -2, mark );

                pushValueToPool( value, mark );

                lua_pop( L, 1 );
            }
        }else
        {
            convertLuaStackValueToString( L, stackindex, value );
            pushValueToPool( value, mark );
        }
    }

    //-----------------------------------------------------------------------
    void LuaManager::convertGlobalVariableToPool( const Str& luaVariable )
    {
        lua_State* L =  getActiveLuaState();

        startAssertStackBalance();
        lua_getglobal( L, luaVariable.c_str() );
        clearVariablePool();
        convertLuaStackValueToPool( -1 );
        lua_pop( L, 1 );
        endAssertStackBalance();
    }

    //-----------------------------------------------------------------------
    void LuaManager::pushValueToPool( const Str& value, const Str& mark)
    {	
        mPoolVariables.push_back( value );
        mPoolVariableMarks.push_back( mark );
    }

    //-----------------------------------------------------------------------
    size_t  LuaManager::getVariableCountInPool(void)
    {
        return mPoolVariables.size();
    }

    //-----------------------------------------------------------------------
    const Str&  LuaManager::getVariableInPool( size_t index )
    {
        return mPoolVariables[index];
    }

    //-----------------------------------------------------------------------
    const Str& LuaManager::getMarkInPool( size_t index )
    {
        return mPoolVariableMarks[index];
    }

    //-----------------------------------------------------------------------
    const Str& LuaManager::getVariableInPool( const Str& mark )
    {
        std::vector<Str>::iterator iter = mPoolVariableMarks.begin();
        size_t n = 0;
        for( ; iter != mPoolVariableMarks.end(); ++iter, ++n )
        {
            if ( (*iter) == mark )
                return mPoolVariables[n];
        }

        return LuaManager_NullVariableInPool;
    }

    //-----------------------------------------------------------------------
    void LuaManager::startAssertStackBalance(void)
    {
#ifdef _DEBUG

        lua_State* L =  getActiveLuaState();

        printStack(L);

        if (mAssertStackCount == 0)
        {
            mStackNumber = lua_gettop( L );
        }
        mAssertStackCount ++;
#endif
    }

    //-----------------------------------------------------------------------
    void LuaManager::endAssertStackBalance(void)
    {
#ifdef _DEBUG

        lua_State* L =  getActiveLuaState();

        printStack(L);

        mAssertStackCount--;

        if ( mAssertStackCount == 0 )
        {
            Int number = lua_gettop( L );
            if ( mStackNumber != number )
            {
                Str errorhInt = "stackBalance assert fail";
                MG_LOG( out_error, ("%s\n"), errorhInt );
                DYNAMIC_ASSERT( false );
            }
        }
#endif
    }

    //-----------------------------------------------------------------------
    void LuaManager::clearVariablePool(void)
    {
        mPoolVariables.clear();
        mPoolVariableMarks.clear();
    }
	//-----------------------------------------------------------------------
	void LuaManager::getIntListFromTable( std::vector<Int>& slt, Int index )
	{
		lua_State* L =  getActiveLuaState();

		if (!lua_istable(L, index))
		{
			MG_LOG( out_error, ("%s error: para not a valid table \n"), __MG_FUNC__ );
		}
		Int number;
		lua_pushnil( L );  
		if (index < 0)
			index -= 1; 
		while (lua_next( L, index) != 0) {
			number = luaL_checkint( L, -1 );
			slt.push_back( number );
			lua_pop( L, 1 );
		}
	}

	// MG_CN_MODIFY
	void LuaManager::setPackManager( IMGExternalPackManager* packManager )
	{
		mPackManager = packManager;
	}

	IMGExternalPackManager* LuaManager::getPackManager()
	{
		return mPackManager;
	}
}