#ifndef __COMMON_H__
#define __COMMON_H__
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
 #define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
 #define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#define SERVER_HOST "localhost"
#define SERVER_PORT 8888
#define MSG_MAX_LEN (512 * 1000)
#define PACKET_SIZE 1024
#define MAX_CLIENTS 5
#define PATH_ROOT "./data/"
//#define PATH_ROOT "/home/pashok/Desktop/bmstu/sem_7/bmstu-NW/lab_03/part_02/server/data"

#endif // __COMMON_H__