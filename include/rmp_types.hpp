#pragma once

typedef unsigned int u32;
typedef unsigned long ul;

static u32 ALLOC_FAILED = 1;
static u32 READ_FAILED = 2;
static u32 WRITE_FAILED = 3;
static u32 FREE_FAILED = 4;
static u32 UNKNOWN_ACTION = 5;
static u32 INVALID_HANDLE = 6;

namespace rmp
{
	typedef long handle;

	// enum req_type
	// {
	// 	ALLOC_PAGES,
	// 	READ_PAGE,
	// 	WRITE_PAGE,
	// 	FREE_PAGES
	// } rmp_req;

	// enum error_type
	// {
	// 	ALLOC_FAILED,
	// 	READ_FAILED,
	// 	WRITE_FAILED,
	// 	FREE_FAILED,
	// 	UNKNOWN_ACTION,
	// 	INVALID_HANDLE
	// } rmp_err;
} // namespace rmp
