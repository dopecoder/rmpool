#ifdef RMP_H
#define RMP_H

namespace rmp
{

	typedef int rmp_handle;
	typedef unsigned long ul;

	struct config
	{
		/*
			TODO: Define a commong connection configuration for both guest and host
			
			Example: IPv4, Protocol, Port, etc.
		*/
	};

	namespace guest
	{
		// Guest
		void rmp_init(struct config);
		char* rmp_alloc(long); // receives a rmp_handle from host
		void rmp_free(rmp_handle);
	}

	namespace host 
	{
		// Host
		void rmp_host_setup(struct config);
		struct config rmp_host_config();
	}
}

#endif
