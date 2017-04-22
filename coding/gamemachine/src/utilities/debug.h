﻿#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "common.h"
#include <map>
#include <string>
#include <set>
BEGIN_NS

// Debug options:
enum DebugOptions
{
	CALCULATE_BSP_FACE,
	POLYGON_LINE_MODE,
	DRAW_ONLY_SKY,
	DRAW_NORMAL, // see DrawNormalOptions

	DEBUG_OPTIONS_END,
};

//Draw normal options
enum DrawNormalOptions
{
	DRAW_NORMAL_OFF,
	DRAW_NORMAL_EYESPACE,
	DRAW_NORMAL_CAMERASPACE,
	DRAW_NORMAL_MAX
};

#define DBG Debug::getDebugger()
#define DBG_SET_INT(key, value) DBG->setInt(key, value)
#define DBG_INT(key) DBG->getInt(key)

class Debug
{
public:
	static Debug* getDebugger();

public:
	void setInt(GMint key, GMint value);
	GMint getInt(GMint key);

private:
	Debug();

private:
	std::map<GMint, GMint> m_ints;
};

inline void format_time(char* in)
{
#ifdef _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	sprintf_s(in, LINE_MAX, "%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

#define f_time(t) char t[LINE_MAX]; format_time(t);
inline void gm_print(const char *format, ...)
{
	f_time(t);
	printf("%s: ", t);
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
}

// debug macros:
#define gm_info gm_print
#define gm_error gm_print
#define gm_warning gm_print

// hooks
#define gm_install_hook(cls, name, funcPtr) { Hooks::install(#cls"_"#name, funcPtr); }
#define gm_hook(cls, name) { Hooks::invoke0(#cls"_"#name)}
#define gm_hook1(cls, name, arg1) { Hooks::invoke1(#cls"_"#name, gm_hook_arg(arg1));}
#define gm_hook2(cls, name, arg1, arg2) { Hooks::invoke2(#cls"_"#name, gm_hook_arg(arg1), gm_hook_arg(arg2));}
#define gm_hook_arg(arg) ((void*)arg)

typedef void(*Hook0)();
typedef void(*Hook1)(void*);
typedef void(*Hook2)(void*, void*);

class Hooks
{
	typedef std::set<Hook0> Hook0_t;
	typedef std::set<Hook1> Hook1_t;
	typedef std::set<Hook2> Hook2_t;

public:
	static void invoke0(const std::string& identifier);
	static void invoke1(const std::string& identifier, void* arg1);
	static void invoke2(const std::string& identifier, void* arg1, void* arg2);
	static void install(const std::string& identifier, Hook0 hook);
	static void install(const std::string& identifier, Hook1 hook);
	static void install(const std::string& identifier, Hook2 hook);

private:
	static Hooks& instance();

private:
	std::map<std::string, Hook0_t> m_hooks0;
	std::map<std::string, Hook1_t> m_hooks1;
	std::map<std::string, Hook2_t> m_hooks2;
};

END_NS
#endif