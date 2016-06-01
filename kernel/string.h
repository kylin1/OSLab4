
#ifndef MULTIPLEPROCESS_STRING_H
#define MULTIPLEPROCESS_STRING_H

#include "const.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            string.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

PUBLIC	void*	memcpy(void* p_dst, void* p_src, int size);
PUBLIC	void	memset(void* p_dst, char ch, int size);
PUBLIC  char*   strcpy(char* p_dst, char* p_src);

#endif //MULTIPLEPROCESS_STRING_H