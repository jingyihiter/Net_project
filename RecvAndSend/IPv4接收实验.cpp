/*
* THIS FILE IS FOR IP TEST
*/
// system support
#include "sysInclude.h"

extern void ip_DiscardPkt(char* pBuffer,int type); //丢弃分组

extern void ip_SendtoLower(char*pBuffer,int length); //交给下层

extern void ip_SendtoUp(char *pBuffer,int length);//交给上层

extern unsigned int getIpv4Address(); //获取本机IPV4地址

// implemented by students

int stud_ip_recv(char *pBuffer,unsigned short length) //接收接口
{
	//获取ip头信息 
	int IpVersion = pBuffer[0]/16;          //ip版本
	//检查ip版本号
	if (IpVersion!= 4)  
	{ 
		ip_DiscardPkt(pBuffer,STUD_IP_TEST_VERSION_ERROR); 
		return 1; 
	} 

	int LengthOfHead = (pBuffer[0] | 0x0) & 0xf;     //IHL
	//检查IHL 
	if (LengthOfHead< 5) 
	{ 
		ip_DiscardPkt(pBuffer,STUD_IP_TEST_HEADLEN_ERROR); 
		return 1; 
	} 

 	int ttl = (int)pBuffer[8]; 			//TTL
	//检查TTL
	if (ttl == 0)  
	{ 
		ip_DiscardPkt(pBuffer,STUD_IP_TEST_TTL_ERROR); 
		return 1; 
	} 

	int DestinationAddress = ntohl(*(unsigned int*)(pBuffer+16));  //目的地址
	//检查目的地址和本机地址是否相同 
	if (DestinationAddress != getIpv4Address()) 
	{ 
		DestinationAddress = DestinationAddress & 0xffffff;
		if(DestinationAddress != 0xffffff)
		{	
			ip_DiscardPkt(pBuffer,STUD_IP_TEST_DESTINATION_ERROR); 
			return 1; 
		}
	} 

	int Checksum = ntohs(*(short unsigned int*)(pBuffer+10)); //校验和
	int sum = 0; 
	unsigned short int localCheckSum = 0; 
	//计算校验和 
	for(int i = 0;i < LengthOfHead *2; i++) 
	{ 
		if(i != 5) 
		{ 
			sum += ((pBuffer[2*i])<<8|pBuffer[2*i+1]); 
			sum %= 65535; 
		} 
		if(LengthOfHead*2 == i+1) 
		{ 
			localCheckSum = (unsigned short int)sum; 
			localCheckSum = ~ localCheckSum;//取反得到校验和
			//检验校验和 
			if(localCheckSum != Checksum ) 
			{ 
				ip_DiscardPkt(pBuffer,STUD_IP_TEST_CHECKSUM_ERROR); 
				return 1; 
			} 
 		}
	} 
	//发送给上层
	ip_SendtoUp(pBuffer,length); 
	return 0; 
}

int stud_ip_Upsend(char *pBuffer,unsigned short len,unsigned int srcAddr,
				   unsigned int dstAddr,byte protocol,byte ttl) //发送接口
{
	char *sendBuffer = new char(len + 20); 
	memset(sendBuffer, 0, len+20); 
	//填入各种IP头信息 
	sendBuffer[0] = 0x45; //高字节4为IP版本号，低字节5为IHL
	unsigned short int lenOfHead = htons(len + 20); 
	memcpy(sendBuffer + 2, &lenOfHead , sizeof(unsigned short int)); 
	sendBuffer[8] = ttl; 
	sendBuffer[9] = protocol; 

	unsigned int source = htonl(srcAddr); 
	unsigned int Destination= htonl(dstAddr); 
	memcpy(sendBuffer + 12, &source , sizeof(unsigned int)); 
	memcpy(sendBuffer + 16, &Destination, sizeof(unsigned int)); 
	//计算校验和
	int sum = 0; 
	unsigned short int localCheckSum = 0,localField; 
	for(int i = 0;i < 10;i ++) 
	{ 	localField= (sendBuffer[i*2]<<8 | sendBuffer[i*2+1]); 
		sum = sum + localField;
		sum %= 65535;
		if(i==9)
		{
			localCheckSum = htons(~(unsigned short int)sum); 	
		} 
	} 
	memcpy(sendBuffer + 10, &localCheckSum, sizeof(unsigned short int)); 
	memcpy(sendBuffer + 20, pBuffer, len); 
	//发送 
	ip_SendtoLower(sendBuffer,len+20); 
	return 0;
}








































