

#include "syCwmpCommon.h"
#include "syCwmpManagement.h"
#include "../thirdLib/pcap/pcap.h"
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>

//#define FAILED 					-1
//#define SUCCESS 				0
//#define DPrint(...) 			printf(__VA_ARGS__)
//#define JUST_FOR_TEST

#define IN
#define OUT
#define MAX_PACKET_SIZE 		65535
#define TEST_FILE_PATH 			"/data/symedia/"
#define MAX_SIZE				100000000

/**/
typedef struct mac_header
{
    u_char dstmacaddress[6];
    u_char srcmacaddress[6];
    u_short type;
} mac_header;

/**/
typedef struct ip_address
{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
} ip_address;

/*自定义IP首部结构体*/
typedef struct ip_header
{
    u_char ver_ihl;
    u_char tos;
    u_short tlen;
    u_short identification;
    u_short flags_fo;
    u_char ttl;
    u_char proto;
    u_short crc;
    ip_address saddr;
    ip_address daddr;
} ip_header;

/*自定义TCP首部结构体*/
typedef struct tcp_header
{
    u_short sourport;
    u_short destport;
    unsigned int sequnum;
    unsigned int acknowledgenum;
    u_short headerlenandflag;
    u_short windowsize;
    u_short checksum;
    u_short urgentpointer;
} tcp_header;

/*自定义UDP首部结构体*/
typedef struct udp_header
{
    u_short sourport;
    u_short destport;
    u_short length;
    u_short checksum;
} udp_header;


/*标准定义IP首部结构体*/
typedef struct _iphdr
{
    unsigned char h_lenver; 		//4位首部长度+4位IP版本号
    unsigned char tos; 				//8位服务类型TOS
    unsigned short total_len; 		//16位总长度（字节）
    unsigned short ident; 			//16位标识
    unsigned short frag_and_flags; 	//3位标志位
    unsigned char ttl; 				//8位生存时间 TTL
    unsigned char proto; 			//8位协议 (TCP, UDP 或其他)
    unsigned short checksum; 		//16位IP首部校验和
    unsigned int sourceIP; 			//32位源IP地址
    unsigned int destIP; 			//32位目的IP地址
} IP_HEADER;

pcap_t *handle = NULL;
char fname[256] = {0};

extern void syKillExistProcess(char *processName);
extern syAcsCpeParamStruct gsyAcsCpeParamStru;
extern time_t gSyCPEStartTime;

void* loopBreak(void *data);
int getFileName(IN const char *path, IN int size, OUT char *filePath);
int findDevs(IN char *errbuf, IN int pFlag, OUT pcap_if_t **alldevs, OUT char **dev);
void ip_analyse(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content);
void* syGetPcapThread(void *data);
void* loopBreakLOG(void *data);
void* syGetLogThread(void *data);
void syKillExistProcess(char *processName);
void runPcapAndLogcat(void);

void ip_analyse(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content)
{
#ifdef JUST_FOR_TEST /*this is for test*/
    ip_header *ipheader;
    string reader;
    mac_header *macheader;

    macheader = (struct mac_header *)packet_content;
    DPrint("SrcMac:%02x:%02x:%02x:%02x:%02x:%02x\n",
           macheader->srcmacaddress[0],
           macheader->srcmacaddress[1],
           macheader->srcmacaddress[2],
           macheader->srcmacaddress[3],
           macheader->srcmacaddress[4],
           macheader->srcmacaddress[5]);
    DPrint("DstMac:%02x:%02x:%02x:%02x:%02x:%02x\n",
           macheader->dstmacaddress[0],
           macheader->dstmacaddress[1],
           macheader->dstmacaddress[2],
           macheader->dstmacaddress[3],
           macheader->dstmacaddress[4],
           macheader->dstmacaddress[5]);

    ipheader = (struct ip_header *)&packet_content[sizeof(struct mac_header)];
    DPrint("SrcIP:%d.%d.%d.%d\n",	ipheader->saddr.byte1,
           ipheader->saddr.byte2,
           ipheader->saddr.byte3,
           ipheader->saddr.byte4);
    DPrint("DstIP:%d.%d.%d.%d\n",	ipheader->daddr.byte1,
           ipheader->daddr.byte2,
           ipheader->daddr.byte3,
           ipheader->daddr.byte4);

    /*	TCP:6
    	UDP:17
    	...
    */
#endif

    /*保存抓包数据到指定文件*/
    pcap_dump(argument, packet_header, packet_content);

    /* 刷新缓冲区 */
    pcap_dump_flush((pcap_dumper_t *)argument);

    return ;
}

int findDevs(IN char *errbuf, IN int pFlag, OUT pcap_if_t **alldevs, OUT char **dev)
{
    if (0 == pFlag)
    {
        *dev = pcap_lookupdev(errbuf);
        if (NULL == dev)
        {
            fprintf(stderr,"Could't find default device:%sn", errbuf);
            return SY_FAILED;
        }
    }
    else
    {
        pcap_if_t *device;
        if (SY_FAILED == pcap_findalldevs(alldevs, errbuf))
        {
            fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
            exit(EXIT_FAILURE);
        }

        int i = 0;
        int num = 0;
        int flag = 0;
        int continueFlag = 0;
        for(device = *alldevs; device != NULL; device = device->next)
        {
            //DPrint("Device name: %s\n", device->name);
            //DPrint("Description: %s\n", device->description);
            num++;
        }
        if (1 == num)
        {
            *dev = device->name;
        }
        else
        {
            while (1)
            {
                char buf[1024] = {0};
                if (0 == flag)
                {
                    flag = 1;
                    for(i = 1, device = *alldevs; device != NULL; device = device->next, i++)
                    {
                        DPrint("<%d> [ Device name: %7s| Description: %s ]\n", i, device->name, device->description);
                    }
                }
                printf("which interface do you want usr:");
                fgets(buf, sizeof(buf), stdin);
                if (strlen(buf) > 1)
                {
                    buf[strlen(buf) - 1] = 0;
                }
                else
                {
                    continue;
                }

                for (i=0; i<strlen(buf); i++)
                {
                    if (buf[i] <= '0' || buf[i] > '9')
                    {
                        continueFlag = 1;
                        break;
                    }
                }

                if ((atoi(buf) > num) || (1 == continueFlag))
                {
                    continueFlag = 0;
                    DPrint("input number is error!\n");
                    continue;
                }
                num = atoi(buf);
                break;
            }
            for(i = 1, device = *alldevs; device != NULL, i<num-1; device = device->next, i++) {}
            *dev = device->name;
        }
    }
    return SY_SUCCESS;
}

int getFileName(IN const char *path, IN int size, OUT char *filePath)
{
    char localFilePath[1024] = {0};
    char filename[256] = {0};
    time_t currentTime = 0L;
    struct tm *cTime = NULL;
    currentTime = time(NULL);
    cTime = localtime(&currentTime);
    snprintf(filename, sizeof(filename), "%d%02d%02d%02d%02d%02d.pcap",
             cTime->tm_year+1900,
             cTime->tm_mon+1,
             cTime->tm_mday,
             cTime->tm_hour,
             cTime->tm_min,
             cTime->tm_sec);
    if (SY_FAILED == access(TEST_FILE_PATH, F_OK))
    {
        DPrint("folder is not exist.\n");
        DPrint("mkdir(%s) result:%d\n", TEST_FILE_PATH, mkdir(TEST_FILE_PATH, 0777));
    }
    snprintf(localFilePath, sizeof(localFilePath), "%s%s", TEST_FILE_PATH, filename);
    strncpy(filePath, localFilePath, size);
    DPrint("filename:%s, localFilePath:%s, filePath:%s\n",
           filename,
           localFilePath,
           filePath);
    return SY_SUCCESS;
}

void* loopBreak(void *data)
{
    DPrint("---->\n");
    int size = MAX_SIZE;
    int count = 0;

    while (1)
    {
        struct stat lStat;
        int ret = stat(fname, &lStat);
        if (SY_FAILED == ret)
        {
            count++;
            DPrint("stat failed, wait for pcap began...\n");
            if (count >= 3)
            {
                DPrint("pcap erorr, exit.\n");
                pcap_breakloop(handle);
                break;
            }
            sleep(2);
        }
        else if (lStat.st_size > size)
        {
            pcap_breakloop(handle);
            break;
        }
        DPrint("fileSize:%lld, target:%d\n", lStat.st_size, size);
        sleep(3);

#if 0
        FILE *fp = fopen(SY_VALUE_CHANGE_INFORM_FLAG_1, "wb");
        if (NULL == fp)
        {
            DPrint("open %s failed.\n", SY_VALUE_CHANGE_INFORM_FLAG_1);
        }
        else
        {
            fclose(fp);
            DPrint("create %s success.\n", SY_VALUE_CHANGE_INFORM_FLAG_1);
        }
#endif
    }
    DPrint("<----\n");
    return NULL;
}

//int main(int argc, char *argv[])
void* syGetPcapThread(void *data)
{
    DPrint("---->\n");
    pcap_t *descr = NULL;
    struct bpf_program fp;
    char filter_exp[1024] = {0};/*设置过滤规则，只获取tcp和udp报文*/
    bpf_u_int32 net;
    pcap_if_t *alldevs = NULL;
    int ret = -1;
    int size = MAX_SIZE;
    
    DPrint("wait for init...\n");
    /* 等待初始化完成拿到网管地址以后再开始，这样可以根据网管地址来设置
    过滤规则,这样抓包会小很多，也方便分析 */
    while (0 == gSyCPEStartTime)
    {
        usleep(400*1000);
    }

    DPrint("URL:%s\n", gsyAcsCpeParamStru.URL);
    if (NULL != gsyAcsCpeParamStru.URL)
    {
        char buf[512] = {0};
        sscanf(gsyAcsCpeParamStru.URL, "%*[^//]//%[^:]:", buf);
        snprintf(filter_exp, sizeof(filter_exp), "host %s", buf);
        DPrint("buf:%s, filter_exp:%s\n", buf, filter_exp);
    }

    char *dev = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
    memset(errbuf, 0x00, PCAP_ERRBUF_SIZE);

#if 0 /*选择网卡设备的函数，一般用pcap_lookupdev获取默认设备*/
    if (FAILED == findDevs(errbuf, 1, &alldevs, &dev))
    {
        DPrint("find dev failed\n");
        exit(-1);
    }
#else
    dev = pcap_lookupdev(errbuf);
#endif

    /*	1) 对应的网卡
    	2) 定义捕获数据的最大值
    	3）设置网卡模式，true混杂， flase不混杂
    	4）超时时间（0表示不超时）
    	5）出错信息的存储BUFF*/
    descr = pcap_open_live(dev, MAX_PACKET_SIZE, 0, 1000, errbuf);
    if (NULL == descr)
    {
        DPrint("Couldn't open device %s:%s\n", dev,errbuf);
        return NULL;
    }
    handle = descr;

    /*	1)pcap_open_live获取到的pcap_t句柄
    	2)struct bpf_program指针，本函数填充
    	3)需要编译的字符串
    	4)控制代码的优化
    	5)本地网络的网络掩码
    */
    if (SY_FAILED == pcap_compile(descr, &fp, filter_exp, 0, net))
    {
        DPrint("Couldn't install parse filter %s:%s\n", filter_exp, pcap_geterr(descr));
        return NULL;
    }

    if (SY_FAILED == pcap_setfilter(descr, &fp))
    {
        DPrint("set filter failed\n");
        return NULL;
    }

    /* 定义输出文件 */
    pcap_dumper_t* out_pcap;
#if 0
    char localFilePath[256] = {0};
    memset(localFilePath, 0x00, sizeof(localFilePath));
    getFileName(TEST_FILE_PATH, sizeof(localFilePath), localFilePath);
    strncpy(fname, localFilePath, sizeof(fname));
    DPrint("filename:%s\n", localFilePath);
#else
    if (SY_FAILED == access(TEST_FILE_PATH, F_OK))
    {
        DPrint("folder is not exist.\n");
        DPrint("mkdir(%s) result:%d\n", TEST_FILE_PATH, mkdir(TEST_FILE_PATH, 0777));
    }

    char filename[] = {TEST_FILE_PATH"pcap.pcap"};
    char fileBackup[] = {TEST_FILE_PATH"pcapBackup.pcap"};
    char cmd[1024] = {0};
    FILE *fileFp = fopen(filename, "rb");
    if (NULL != fileFp)
    {
        fclose(fileFp);
        fileFp = NULL;
        snprintf(cmd, sizeof(cmd), "mv %s %s", filename, fileBackup);
        DPrint("system(%s) result:%d\n", cmd, system(cmd));
    }
    strncpy(fname, filename, sizeof(fname));
#endif
    out_pcap = pcap_dump_open(descr, fname);
    if (!out_pcap)
    {
        DPrint("failed\n");
        return NULL;
    }

    /*创建线程控制抓包的大小，不能无限制的抓下去，会导致盒子存储不够挂掉*/
    pthread_t lTid;
    if (pthread_create(&lTid, NULL, loopBreak, (void*)&size) < 0)
    {
        DPrint("create thread failed\n");
        return NULL;
    }
    else
    {
        pthread_detach(lTid);
    }
    usleep(100*1000);

    /*	1)打开的网卡指针
    	2)指定捕获的数据包个数（-1则无限循环）
    	3)回调函数
    	pcap_callback(u_char* argument,const struct pcap_pkthdr* packet_header,const u_char* packet_content)
    	struct pcap_pkthdr {
    		struct timeval ts; 		// 时间戳
    		bpf_u_int32 caplen; 	// 已捕获部分的长度
    		bpf_u_int32 len; 		// 该包的脱机长度
    	};
    	4)留给user用户使用的参数
    */
    pcap_loop(descr, -1, ip_analyse, (u_char *)out_pcap);

    /* 刷新缓冲区 */
    pcap_dump_flush(out_pcap);

    /* 关闭资源 */
    pcap_close(descr);
    pcap_dump_close(out_pcap);

    /* 不再需要设备列表了，释放它 */
    pcap_freealldevs(alldevs);
    DPrint("<----\n");
    return NULL;
}

void* loopBreakLOG(void *data)
{
    DPrint("---->\n");
    int size = MAX_SIZE;
    int count = 0;
    char filename[1024] = {0};

    strncpy(filename, (char*)data, sizeof(filename));
    DPrint("filename:%s\n", filename);
    while (1)
    {
        struct stat lStat;
        int ret = stat(filename, &lStat);
        if (SY_FAILED == ret)
        {
            count++;
            DPrint("stat failed, wait for logcat began...\n");
            if (count >= 3)
            {
                DPrint("logcat error, exit.\n");
                syKillExistProcess("logcat");
                break;
            }
            sleep(2);
        }
        else if (lStat.st_size > size)
        {
            syKillExistProcess("logcat");
            break;
        }
        DPrint("fileSize:%lld, target:%d\n", lStat.st_size, size);
        sleep(5);
    }
    return NULL;
}

void* syGetLogThread(void *data)
{
    DPrint("---->\n");
    time_t currentTime = 0L;
    struct tm *cTime = NULL;
    currentTime = time(NULL);
    cTime = localtime(&currentTime);

    if (SY_FAILED == access(TEST_FILE_PATH, F_OK))
    {
        DPrint("folder is not exist.\n");
        DPrint("mkdir(%s) result:%d\n", TEST_FILE_PATH, mkdir(TEST_FILE_PATH, 0777));
    }

    char filename[] = {TEST_FILE_PATH"log.log"};
    char fileBackup[] = {TEST_FILE_PATH"logBackup.log"};
    char cmd[1024] = {0};
    FILE *fp = fopen(filename, "rb");
    if (NULL != fp)
    {
        fclose(fp);
        fp = NULL;
        snprintf(cmd, sizeof(cmd), "mv %s %s", filename, fileBackup);
        DPrint("system(%s) result:%d\n", cmd, system(cmd));
    }

    pthread_t tid;
    if (0 == pthread_create(&tid, NULL, loopBreakLOG, (void*)filename))
    {
        DPrint("create thread to console logcat success.\n");
        pthread_detach(tid);
    }

    memset(cmd, 0x00, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "logcat -vtime>%s", filename);
    DPrint("system(%s) result:%d\n", cmd, system(cmd));

    DPrint("<----\n");
    return NULL;
}

#if 0
void syKillExistProcess(char *processName)
{
    char cmd[64] = {0};
    char *pingbuf = (char *)malloc(1024);
    FILE *fd;
    int pid;
    sprintf(cmd, "ps |grep %s",processName);
    if ((fd = popen(cmd, "r")) != NULL)
    {
        while (fgets(pingbuf, 1024, fd))
        {
            DPrint("%s", pingbuf);
            while( *pingbuf > '9' || *pingbuf < '0')
            {
                pingbuf++;
            }
            pid = atoi(pingbuf);
            DPrint("pid = %d", pid);
            kill(pid, SIGTERM);
        }
    }
    pclose(fd);
}
#endif

void runPcapAndLogcat(void)
{
    DPrint("---->\n");
    FILE *fp = NULL;
    /*以后抓log可以不用编译版本来控制
    可以在/data/data/下创建文件来控制是否
    需要启动网管就自动抓log*/
    fp = fopen("/data/data/isNeedLOGCAT", "rb");
    if (NULL != fp)
    {
        pthread_t lPid1;
        if (!pthread_create(&lPid1, NULL, syGetLogThread, NULL))
        {
            DPrint("logcat threadID:%lu\n", lPid1);
            pthread_detach(lPid1);
        }
        else
        {
            DPrint("handle logcat error\n");
        }
        fclose(fp);
        fp = NULL;
    }

    /*以后抓包可以不用编译版本来控制
    可以在/data/data/下创建文件来控制是否
    需要网管启动就自动抓包*/
    fp = fopen("/data/data/isNeedPCAP", "rb");
    if (NULL != fp)
    {
        pthread_t lPid2;
        if (!pthread_create(&lPid2, NULL, syGetPcapThread, NULL))
        {
            DPrint("pcap threadID:%lu\n", lPid2);
            pthread_detach(lPid2);
        }
        else
        {
            DPrint("handle pcap error\n");
        }
        fclose(fp);
        fp = NULL;
    }
    DPrint("<----\n");
}



