/* varargs.h for MicroGnuEmacs 2a.  This one will work on systems that	*/
/* the non-varargs version of mg 1 did.					*/
/* based on the one I wrote for os9/68k .  I did not look at the bsd code. */

/* by Robert A. Larson */

/* assumptions made about how arguments are passed:			*/
/*	arguments are stored in a block of memory with no padding between. */
/*	The first argument will have the lowest address			*/

/* varargs is a "portable" way to write a routine that takes a variable */
/* number of arguements.  This implemination agrees with both the 4.2bsd*/
/* and Sys V documentation of varargs.  Note that just because varargs.h*/
/* is used does not mean that it is used properly.			*/

#define va_dcl		unsigned va_alist;

typedef	char *va_list;

#define	va_start(pvar)		((pvar) = (char *)&va_alist)

#define va_arg(pvar,type)	(((pvar)+=sizeof(type)),*(((type *)(pvar)) - 1))

#define va_end(pvar)		/* va_end is simple */
