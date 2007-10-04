/* For Jon Zeeff's dbm-clone for news */

typedef struct {
	char	*dptr;
	int	dsize;
} dbzdatum;

extern dbzdatum	dbzfetch();
extern int	dbzdbminit(), dbzdbmclose(), dbzstore();
