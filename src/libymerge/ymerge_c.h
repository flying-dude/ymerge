// exposing ymerge functionality as a C interface

#include "alpm_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ymerge --sync */
int ymerge_sync();

/**
 * Attempt queuing a package in ymerge. Returns true, if the package exists.
 * Writes binary dependencies into **yman_deps.
 */
int ymerge_process_pkg(alpm_list_t *dbs, const char *pkg);

#ifdef __cplusplus
}
#endif
