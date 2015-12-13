#include <sys/types.h>
#include <libelftc.h>

const char *
elftc_version(void)
{
	return "elftoolchain HEAD LiteBSD r" GITCOUNT;
}
