#ifndef PACKUNPACK_H
#define PACKUNPACK_H


/* function prototypes
 * - intended to be public to application programs
*/
struct argmessage_message* packunpack_dummypayload();
struct argmessage_message* packunpack_packonestring(char* string0);
struct argmessage_message* packunpack_packoneint(int int0);
struct argmessage_message* packunpack_packtwoint(int int0, int int1);
struct argmessage_message* packunpack_packthreeint(int int0, int int1, int int2);
struct argmessage_message* packunpack_packfourint(int int0, int int1, int int2, int int3);
struct argmessage_message* packunpack_packfiveint(int int0, int int1, int int2, int int3, int int4);
struct argmessage_message* packunpack_packsixint(int int0, int int1, int int2, int int3, int int4, int int5);
void packunpack_callnoargs(int adapter_idx, void (*func)(), struct argmessage_message* message);
struct argmessage_message* packunpack_callnoargswithreturn(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callonestring(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callbytearray(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_calloneint(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_calltwoint(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callthreeint(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callfourint(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callfiveint(int adapter_idx, void (*func)(), struct argmessage_message* message);
void packunpack_callsixint(int adapter_idx, void (*func)(), struct argmessage_message* message);

#endif /* PACKUNPACK_H */
