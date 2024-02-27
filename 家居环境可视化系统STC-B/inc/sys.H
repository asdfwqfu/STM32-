#ifndef	_sys_h_
#define _sys_h_

typedef struct                     //系统性能评估参数,每秒更新一次
  { unsigned long MainLoops;            //SysPerformance.MainLoops            每秒主循环次数（应大于1000以上）
		unsigned char PollingMisses;        //SysPerformance.PollingMisses        每秒轮询丢失次数（理想值为0。如果有非0情况，说明用户程序或算法存在问题）
	} struct_SysPerF;
	
extern void MySTC_Init();             //sys初始化函数
extern void MySTC_OS();               //sys调度函数，应在while（1）循环中                                     
extern void SetEventCallBack(char event, void *(user_callback));  //加载"事件"用户回调函数.
extern struct_SysPerF GetSysPerformance(void); 
		
enum event{enumEventSys1mS,                      //系统1mS事件
	         enumEventSys10mS,                     //系统10mS事件
           enumEventSys100mS,                    //系统100mS事件
           enumEventSys1S,                       //系统1S事件
           enumEventKey,                         //按键事件
           enumEventHall,                        //hall传感器事件
           enumEventVib,                         //振动传感器事件
           enumEventNav,                         //导航按键事件“
           enumEventXADC,                        //扩展接口上新的AD值事件
           enumEventUart1Rxd,                    //串口1上收到一个符合格式定义的事件
	         enumEventUart2Rxd,                    //串口2上收到一个符合格式定义（"数据包格式"、或"ModBus协议+数据包格式"）的事件。    （2022年5月2日新增） 
	         enumEventIrRxd                        //红外Ir上收到一个数据包
          };                         
	
#endif