
/* -------- client to server command bytes */

/* start sending a datastream */
#define ALDL_COMM_STREAM 0x20

/* send the stream definition header */
#define ALDL_COMM_SENDHEADER 0x21

/* send a server status byte */
#define ALDL_COMM_SENDSTATUS 0x22

/* -------- server to client command bytes */

/* the 'command accepted' reply */
#define ALDL_COMM_OK 0x10

/* the 'command not accepted' reply */
#define ALDL_COMM_ERR 0x11

/* the server is full, please retry */
#define ALDL_COMM_SERVERFULL 0x12

/* prefix for a valid datastream */
#define ALDL_COMM_OK 0x13

/* the aldl stream is in a disconnected state */
#define ALDL_COMM_DOWN 0x14

