// exposing ymerge functionality as a C interface

#ifdef __cplusplus
extern "C" {
#endif

// ymerge --sync
int ymerge_sync();

// let yman query, if ymerge provides a certain package
int ymerge_package_exists(const char * name);

#ifdef __cplusplus
}
#endif
