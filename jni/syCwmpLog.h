#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <poll.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>

#include "syCwmpCommon.h"
#include "syCwmpSocket.h"

#define MSG_LEN             4096
#define SUCCESS             1
#define FAILURE             -1




typedef struct {
	int     cmd;    //命令值
	syOpYype type;   //命令值不再区分读和写，通过type来区分读写，0--读，1--写；
	int     len;    //msg长度
	char    msg[MSG_LEN];
}QOSMSG;

#define LOG_FILE_PATH		"/data/"

typedef int (*setIptvData)(QOSMSG*);

int runLogcat(QOSMSG* pMsg);

static void* MsgStartTimeThread(void *data);

static void* MsgStartLogcatThread(void *data);

static void* MsgLogcatThread(void *data);

static void* MsgLogcatUploadThread(void *data);

int GetMAC(char* pszMAC, char mark);
static int GetCurrentTime(int type, char *stime);
int sySendMsgToTms(const char* msg, int msgLen);









