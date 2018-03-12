

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

/*�Զ���IP�ײ��ṹ��*/
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

/*�Զ���TCP�ײ��ṹ��*/
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

/*�Զ���UDP�ײ��ṹ��*/
typedef struct udp_header
{
    u_short sourport;
    u_short destport;
    u_short length;
    u_short checksum;
} udp_header;


/*��׼����IP�ײ��ṹ��*/
typedef struct _iphdr
{
    unsigned char h_lenver; 		//4λ�ײ�����+4λIP�汾��
    unsigned char tos; 				//8λ��������TOS
    unsigned short total_len; 		//16λ�ܳ��ȣ��ֽڣ�
    unsigned short ident; 			//16λ��ʶ
    unsigned short frag_and_flags; 	//3λ��־λ
    unsigned char ttl; 				//8λ����ʱ�� TTL
    unsigned char proto; 			//8λЭ�� (TCP, UDP ������)
    unsigned short checksum; 		//16λIP�ײ�У���
    unsigned int sourceIP; 			//32λԴIP��ַ
    unsigned int destIP; 			//32λĿ��IP��ַ
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

    /*����ץ�����ݵ�ָ���ļ�*/
    pcap_dump(argument, packet_header, packet_content);

    /* ˢ�»����� */
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
    char filter_exp[1024] = {0};/*���ù��˹���ֻ��ȡtcp��udp����*/
    bpf_u_int32 net;
    pcap_if_t *alldevs = NULL;
    int ret = -1;
    int size = MAX_SIZE;
    
    DPrint("wait for init...\n");
    /* �ȴ���ʼ������õ����ܵ�ַ�Ժ��ٿ�ʼ���������Ը������ܵ�ַ������
    ���˹���,����ץ����С�ܶ࣬Ҳ������� */
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

#if 0 /*ѡ�������豸�ĺ�����һ����pcap_lookupdev��ȡĬ���豸*/
    if (FAILED == findDevs(errbuf, 1, &alldevs, &dev))
    {
        DPrint("find dev failed\n");
        exit(-1);
    }
#else
    dev = pcap_lookupdev(errbuf);
#endif

    /*	1) ��Ӧ������
    	2) ���岶�����ݵ����ֵ
    	3����������ģʽ��true���ӣ� flase������
    	4����ʱʱ�䣨0��ʾ����ʱ��
    	5��������Ϣ�Ĵ洢BUFF*/
    descr = pcap_open_live(dev, MAX_PACKET_SIZE, 0, 1000, errbuf);
    if (NULL == descr)
    {
        DPrint("Couldn't open device %s:%s\n", dev,errbuf);
        return NULL;
    }
    handle = descr;

    /*	1)pcap_open_live��ȡ����pcap_t���
    	2)struct bpf_programָ�룬���������
    	3)��Ҫ������ַ���
    	4)���ƴ�����Ż�
    	5)�����������������
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

    /* ��������ļ� */
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

    /*�����߳̿���ץ���Ĵ�С�����������Ƶ�ץ��ȥ���ᵼ�º��Ӵ洢�����ҵ�*/
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

    /*	1)�򿪵�����ָ��
    	2)ָ����������ݰ�������-1������ѭ����
    	3)�ص�����
    	pcap_callback(u_char* argument,const struct pcap_pkthdr* packet_header,const u_char* packet_content)
    	struct pcap_pkthdr {
    		struct timeval ts; 		// ʱ���
    		bpf_u_int32 caplen; 	// �Ѳ��񲿷ֵĳ���
    		bpf_u_int32 len; 		// �ð����ѻ�����
    	};
    	4)����user�û�ʹ�õĲ���
    */
    pcap_loop(descr, -1, ip_analyse, (u_char *)out_pcap);

    /* ˢ�»����� */
    pcap_dump_flush(out_pcap);

    /* �ر���Դ */
    pcap_close(descr);
    pcap_dump_close(out_pcap);

    /* ������Ҫ�豸�б��ˣ��ͷ��� */
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
    /*�Ժ�ץlog���Բ��ñ���汾������
    ������/data/data/�´����ļ��������Ƿ�
    ��Ҫ�������ܾ��Զ�ץlog*/
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

    /*�Ժ�ץ�����Բ��ñ���汾������
    ������/data/data/�´����ļ��������Ƿ�
    ��Ҫ�����������Զ�ץ��*/
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



