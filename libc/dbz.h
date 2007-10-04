/* For Jon Zeeff's dbm-clone for news */

typedef struct {
	char	*dptr;
	int		dsize;
} datum;

extern datum	fetch();
extern int	dbminit(), dbmclose(), store();
