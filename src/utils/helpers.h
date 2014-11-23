#ifndef __HELPER_H__
#define __HELPER_H__

int dumps_request(char* domain, char* buf);

char* loads_request(char* buf);

int dumps_response(char* domain, char* ip, char* buf);

char* loads_response(char* buf);


#endif
