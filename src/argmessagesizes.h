#ifndef ARGMESSAGESIZES_H
#define ARGMESSAGESIZES_H

// TODO: move these declarations to ./configure
/* of the below:
 * user function indexes start at 0, system function inedexes have high numbers
 * because user is invited to draw up an enum of function names adn indexes, 
 * which defaults to zero based
*/
#define ARGMESSAGE_MAX_USER_FUNCTION_NAMES 100
#define ARGMESSAGE_MAX_SERVER_FUNCTION_NAMES 28
#define ARGMESSAGE_SERVER_FUNCTIONS_BASE ARGMESSAGE_MAX_USER_FUNCTION_NAMES
#define ARGMESSAGE_MAX_FUNCTION_NAMES ARGMESSAGE_MAX_USER_FUNCTION_NAMES + \
          ARGMESSAGE_MAX_SERVER_FUNCTION_NAMES

#define ARGMESSAGE_ADAPTERS_PER_PROXY 4
#define ARGMESSAGE_MAX_RANKS 16
#define ARGMESSAGE_MAX_ADAPTERS_PER_ENGINE ARGMESSAGE_ADAPTERS_PER_PROXY * ARGMESSAGE_MAX_RANKS 

#ifdef QSARGM_OPTIM_02
#define ARGMESSAGE_RECEIVE_BUFFER_BYTES 64
#else
#define ARGMESSAGE_RECEIVE_BUFFER_BYTES 1024
#endif /* QSARGM_OPTIM_02 */

#endif /* ARGMESSAGESIZES_H */
