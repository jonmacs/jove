#define USE_STDIO_H 1
#include "jove.h"
#include "chars.h"
#include "commands.h"
#include "vars.h"

#define	PROC(p)	(cmdproc_t)0	/* discard function pointers */
#include "commands.tab"

#define VAR(v)	NULL, (size_t)0	/* discard variable pointers */
#include "vars.tab"

private int debug = 0;

#define SEARCH jbsearch
extern UnivPtr SEARCH(UnivConstPtr /*key*/, UnivConstPtr /*base*/,
		       size_t /*nmemb*/, size_t /*size*/,

		       int (*compar) ptrproto((UnivConstPtr, UnivConstPtr)));

UnivPtr
jbsearch(key, base, nmemb, size, compar)
UnivConstPtr key, base;
size_t nmemb, size;
int (*compar) ptrproto((UnivConstPtr, UnivConstPtr));
{
	size_t n = nmemb;
	const char *midp, *lop = (const char *) base;
	int v;
	while (n > 0) {
		midp = lop + (n >> 1)*size;
		v = (*compar)(key, midp);
		if (debug) printf("%zu %zu %zu %d\n", (lop - (const char *)base)/size, (midp - (const char *)base)/size, n >> 1, v);
		if (v == 0) {
			return (UnivPtr)midp;
		}
		if (v > 0) {
		    lop = midp + size;
		    n--;
		}
		n >>= 1;
	}
	return NULL;
}

int comcmp(p1, p2)
UnivConstPtr p1, p2;
{
	const struct cmd *c1 = (const struct cmd *) p1;
	const struct cmd *c2 = (const struct cmd *) p2;
	if (debug) printf("\"%s\" \"%s\"\n", c1->Name, c2->Name);
	return strcmp(c1->Name, c2->Name);
}

int varcmp(p1, p2)
UnivConstPtr p1, p2;
{
	const struct variable *v1 = (const struct variable *) p1;
	const struct variable *v2 = (const struct variable *) p2;
	if (debug) printf("\"%s\" \"%s\"\n", v1->Name, v2->Name);
	return strcmp(v1->Name, v2->Name);
}

int
main(int argc, char **argv)
{
    int i, ktype;
    const struct cmd *cp;
    const struct variable *vp;
    printf("%zu cmds, %zu vars\n", elemsof(commands), elemsof(variables));
    if (commands[elemsof(commands)-1].Name != NULL) {
	printf("last element of commands not NULL\n");
	exit(1);
    }
    if (variables[elemsof(variables)-1].Name != NULL) {
	printf("last element of variables not NULL\n");
	exit(1);
    }
    if(getenv("DEBUG")) debug = atoi(getenv("DEBUG"));
    if (argc < 3 || (ktype = atoi(argv[1])) < 1 || ktype > 2) {
	fprintf(stderr, "Usage: %s [1|2] cmd_or_varname\n\t1 means cmd, 2 means var\n", argv[0]);
	exit(2);
    }
    for (i = 2; i < argc; i++) {
	    if (ktype == 1) {
		    struct cmd ckey;
		    ckey.Name = argv[i];
		    cp = (const struct cmd *)SEARCH((UnivConstPtr)&ckey,
					(UnivConstPtr)commands,
					elemsof(commands)-1,
					sizeof(struct cmd), comcmp);
		    if (cp == NULL) {
			    printf("did not find %d \"%s\" in commands\n", i, argv[i]);
		    } else {
			    printf("found %d \"%s\" \"%s\" in commands\n", i, argv[i], cp->Name);
		    }
	    } else {
	    	    struct variable vkey;
		    vkey.Name = argv[i];
		    vp = (const struct variable *)SEARCH((UnivConstPtr)&vkey,
						(UnivConstPtr)variables,
						elemsof(variables)-1,
						sizeof(struct variable), varcmp);
		    if (vp == NULL) {
			    printf("did not find %d \"%s\" in vars\n", i, argv[i]);
		    } else {
			    printf("found %d \"%s\" \"%s\" in vars\n", i, argv[i], vp->Name);
		    }
	    }
    }
}
