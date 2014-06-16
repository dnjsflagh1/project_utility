/******************************************************************************/
#ifndef _MGTIMEOP_H_
#define _MGTIMEOP_H_
/******************************************************************************/

/******************************************************************************/
namespace MG
{

	/******************************************************************************/
	//MGTime 时间操作类
	/******************************************************************************/
	class MGTimeOp
	{
	public:
		static	void		syncTime(U64 xStdTime);								//同步时间，主要用于客户端，同步服务器的时间，xStdTime为服务器发来的时间
		static	Bool		inSameHour(U64 xTime);								//判断当前时间和目标时间在一个小时之内，不是间隔长度，比如都在8:00于9:00之间就同
		static	Bool		inSameDay(U64 xTime);								//判断当前时间和目标时间在同一天，不是间隔长度
		static	Bool		inSameWeek(U64 xTime);								//判断是否在同一个星期内
		static	Bool		inSameMonth(U64 xTime);								//判断是否在同一个月内

		static	U64			getCurrTimestamp();									//获取当前时刻
		static	Bool		timePass(U64 xCheckTime);							//判断当前时刻是否已经过了目标时刻,true表示过了
		static	U64			getFutureTime(I32 Hours,I32 Minutes,I32 Seconds);	//获取将来时刻，比如n小时x分钟后的时刻
		static	U64			getFutureTime(I32 nDays);							//获取将来时刻,用天数做偏移
		static	U64			getFutureTime(U64 xStart,U32 dwOffset);				//偏移时刻
		static	U64			getTimeInToday(I32 Hour,I32 Minute,I32 Second);		//获取今天内时刻，用xx小时xx分钟xx秒来表示,比如今天 4:00:00这个时刻 
		static	U64			getTimeInToday(U32 dwOffset);						//获取今天内时刻，用秒为偏移
		static	U64			createtimestamp(U32 wYear,U8 bMonth,U8 bDay,U8 bHour = 0,U8 bMinute = 0,U8 bSecond = 0);//创建时刻
		static	U8			getWeekDay();										//获取星期编号，1表示星期一,7表示星期日
		static	U8			getMonthDay();										//获取月内日子编号，1表示1号
		static	I32			getDaysBeforeDeadline(U64 xDeadline);				//获取距目标时刻的期限，天为单位
		static	I32			getHoursBeforeDeadline(U64 xDeadline);				//小时为单位
		static	U32			getCurrTick();										//获取当前tick
		static	void		setSvrTick(U32 dwTick);								//设置服务器端当前的tick，用于客户端于服务器的同步
		static	SYSTEMTIME	getLocalTime(U64 xTime);
	};
}
#endif