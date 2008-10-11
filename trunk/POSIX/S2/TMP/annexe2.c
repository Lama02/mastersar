#define _POSIX_SOURCE 1


/**** TME2 Annexe 2 ****/

void get_user_input_loop ()
{
    int or = 1;
	char buf[1024];

    while (or > 0) 
        if ((or = read(STDIN_FILENO, buf, BUFSZ)) == -1)
			perror("read");
}
