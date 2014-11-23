#ifndef __HELPER_H__
#define __HELPER_H__

/**
 * Serialize a request of given domain name
 * @param  domain The domain name in dns query
 * @param  buf    The buffer that stores the serialized dns query
 * @return        The size in bytes of the serialized dns query
 */
int dumps_request(char* domain, char* buf);

/**
 * Deserialize a dns query
 * @param  buf The buffer that stores the serialized dns query
 * @return     The domain name contained in the dns query
 */
char* loads_request(char* buf);

/**
 * Serialize a response of given domain name and ip
 * @param  domain The domain name in dns response
 * @param  ip     The ip corresponding to the domain name
 * @param  buf    The buffer that stores the serialized dns response
 * @return        The size in bytes of the serialized dns response
 */
int dumps_response(char* domain, char* ip, char* buf);

/**
 * Deserialize a dns response
 * @param  buf The buffer that stores the serialized dns response
 * @return     The ip address contained in the dns response
 */
char* loads_response(char* buf);

#endif
