#ifndef MAIN_H
#define MAIN_H

inline const char * LABEL() { return "Some text"; }
inline const int * ARRAY() { static const int array[] = {0,1,2}; return array; }
const int REQUEST_TIMEOUT_MS = 5000;

#endif // MAIN_H
