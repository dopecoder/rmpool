#ifndef RMPTYPES_HPP
#define RMPTYPES_HPP

typedef unsigned long ul;

typedef int rmp_handle;

enum rmp_req_type
{
	ALLOC_PAGES,
	READ_PAGE,
	WRITE_PAGE,
	FREE_PAGES
};

#endif
