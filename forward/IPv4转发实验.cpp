#include "sysInclude.h"
#include<map>
using std::map;
// system support
extern void fwd_LocalRcv(char *pBuffer, int length);
extern void fwd_SendtoLower(char *pBuffer, int length, unsigned int nexthop);
extern void fwd_DiscardPkt(char *pBuffer, int type);
extern unsigned int getIpv4Address( );
// implemented by students            
map<int,int>mapTable;          //路由表存储  第一个参数 目的地址  第二个参数 下一跳
void stud_Route_Init()  	//初始化路由表
{ 
 mapTable.clear();       //清空map             
 return;
}
void stud_route_add(stud_route_msg *proute)  //路由表中添加项
{
 int DestinationAddress,nextHop;
 DestinationAddress=(ntohl(proute->dest))&(0xffffffff<<(32-htonl(proute->masklen)));
 nextHop=ntohl(proute->nexthop);
 mapTable[DestinationAddress]=nextHop;    //插入
}
int stud_fwd_deal(char *pBuffer, int length)  //处理收到的分组
{
 int IHL=pBuffer[0]&0xf;
 int TTL=(int)pBuffer[8];
 int Head_Checksum=ntohs(*(unsigned short*)(pBuffer+10));
 int Dst_IP=ntohl(*(unsigned*)(pBuffer+16));
 if(TTL<=0) {
  fwd_DiscardPkt(pBuffer,STUD_FORWARD_TEST_TTLERROR); 
  return 1;
 }
 if(Dst_IP==getIpv4Address()) {
  fwd_LocalRcv(pBuffer,length);  //上交分组
  return 0;
 }
 map<int,int>::iterator iter;
 iter = mapTable.find(Dst_IP);
 if(iter!=mapTable.end()){
  char *buffer=new char[length];
   memcpy(buffer,pBuffer,length);
   buffer[8]--;  //ttl减一
   int sum=0,i;
   unsigned short LocalChecksum=0;
   for(i=0;i<2*IHL;i++) {
    if(i!=5){
     sum+=(buffer[2*i]<<8)|(buffer[2*i+1]);
     sum%=65535;
    }
   if(i+1 == 2*IHL){
   LocalChecksum=htons(~(unsigned short)sum); //重新计算校验和
   memcpy(buffer+10,&LocalChecksum,2);
   }
   }
   fwd_SendtoLower(buffer,length,iter->second); //调用下层进行发送处理
   return 0;
 }
 fwd_DiscardPkt(pBuffer,STUD_FORWARD_TEST_NOROUTE);
 return 1;
}
