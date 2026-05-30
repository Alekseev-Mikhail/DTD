#ifndef DUMMY3D_EXCEPTION_H
#define DUMMY3D_EXCEPTION_H

#include <stdbool.h>

#define IS_NULL(ptr) (ptr == NULL ? (llog(ERROR, "Null pointer exception"), true) : false)
#define status bool
#define COMPLETED true
#define FAILED false

#endif //DUMMY3D_EXCEPTION_H
