#ifndef __HELPER_H__
#define __HELPER_H__

/**
 * Serialize a request of given domain name
 * @param  id     The id of the request
 * @param  domain The domain name in dns query
 * @param  buf    The buffer that stores the serialized dns query
 * @return        The size in bytes of the serialized dns query
 */
int dumps_request(int id, char* domain, char* buf);

/**
 * Serialize a response of given domain name and ip. When dimain and ip are
 * NULL, a response with RCODE = 3 will be generated and serialize
 *
 * @param  domain The DNS query message
 * @param  ip     The ip corresponding to the domain name
 * @param  buf    The buffer that stores the serialized dns response
 * @return        The size in bytes of the serialized dns response
 */
int dumps_response(message_t* msg, char* ip, char* buf);

/**
 * Deserialize a dns response
 * @param  buf The buffer that stores the serialized dns response
 * @return     The ip address contained in the dns response. NULL if the
 *             response contains no answer
 */
char* loads_response(char* buf);

#endif
