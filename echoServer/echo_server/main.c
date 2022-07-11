#include "echo_server.h"
#include "ev2.h"

#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>


int init_rlimit()
{
	struct rlimit rli;

	if (getrlimit(RLIMIT_NOFILE, &rli) == 0) {

		if (rli.rlim_max > 65536)
			rli.rlim_cur = 65536;
		else
			rli.rlim_cur = rli.rlim_max;

		setrlimit(RLIMIT_NOFILE, &rli);
	}
    return 0;
}

int main(int argc, char **argv)
{
	int port;
	int i;
	int err;
	int n = 0;
	ev2_loop_t *loop;

	init_rlimit();
	loop = ev2_loop_new();//epoll_create

	for (i = 1; i < argc; ++i) {
	    port = atoi(argv[i]);
	    port &= 0x0000ffff;
	    if (!port) {
	        fprintf(stderr, "invalid args[%d] %s\n", i, argv[i]);
	        ev2_loop_free(loop);
	        return 1;
	    }

		// TODO: List to hold service instances.
		echo_server_t *s = echo_server_new(loop);
		err = echo_server_listen(s, port);
		if (err < 0) {
			ev2_loop_free(loop);
			return 1; 
		}
		n += 1;
	}
	if (n == 0) {
		fprintf(stderr, "no port specified.\n");
		return 1;
	}


	int r=ev2_loop_run(loop);
	if (r < 0)
	{
		printf("·µ»Ø´íÎó\n");
	}

	return 0;
}
