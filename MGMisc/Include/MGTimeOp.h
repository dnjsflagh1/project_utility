/******************************************************************************/
#ifndef _MGTIMEOP_H_
#define _MGTIMEOP_H_
/******************************************************************************/

/******************************************************************************/
namespace MG
{

	/******************************************************************************/
	//MGTime ʱ�������
	/******************************************************************************/
	class MGTimeOp
	{
	public:
		static	void		syncTime(U64 xStdTime);								//ͬ��ʱ�䣬��Ҫ���ڿͻ��ˣ�ͬ����������ʱ�䣬xStdTimeΪ������������ʱ��
		static	Bool		inSameHour(U64 xTime);								//�жϵ�ǰʱ���Ŀ��ʱ����һ��Сʱ֮�ڣ����Ǽ�����ȣ����綼��8:00��9:00֮���ͬ
		static	Bool		inSameDay(U64 xTime);								//�жϵ�ǰʱ���Ŀ��ʱ����ͬһ�죬���Ǽ������
		static	Bool		inSameWeek(U64 xTime);								//�ж��Ƿ���ͬһ��������
		static	Bool		inSameMonth(U64 xTime);								//�ж��Ƿ���ͬһ������

		static	U64			getCurrTimestamp();									//��ȡ��ǰʱ��
		static	Bool		timePass(U64 xCheckTime);							//�жϵ�ǰʱ���Ƿ��Ѿ�����Ŀ��ʱ��,true��ʾ����
		static	U64			getFutureTime(I32 Hours,I32 Minutes,I32 Seconds);	//��ȡ����ʱ�̣�����nСʱx���Ӻ��ʱ��
		static	U64			getFutureTime(I32 nDays);							//��ȡ����ʱ��,��������ƫ��
		static	U64			getFutureTime(U64 xStart,U32 dwOffset);				//ƫ��ʱ��
		static	U64			getTimeInToday(I32 Hour,I32 Minute,I32 Second);		//��ȡ������ʱ�̣���xxСʱxx����xx������ʾ,������� 4:00:00���ʱ�� 
		static	U64			getTimeInToday(U32 dwOffset);						//��ȡ������ʱ�̣�����Ϊƫ��
		static	U64			createtimestamp(U32 wYear,U8 bMonth,U8 bDay,U8 bHour = 0,U8 bMinute = 0,U8 bSecond = 0);//����ʱ��
		static	U8			getWeekDay();										//��ȡ���ڱ�ţ�1��ʾ����һ,7��ʾ������
		static	U8			getMonthDay();										//��ȡ�������ӱ�ţ�1��ʾ1��
		static	I32			getDaysBeforeDeadline(U64 xDeadline);				//��ȡ��Ŀ��ʱ�̵����ޣ���Ϊ��λ
		static	I32			getHoursBeforeDeadline(U64 xDeadline);				//СʱΪ��λ
		static	U32			getCurrTick();										//��ȡ��ǰtick
		static	void		setSvrTick(U32 dwTick);								//���÷������˵�ǰ��tick�����ڿͻ����ڷ�������ͬ��
		static	SYSTEMTIME	getLocalTime(U64 xTime);
	};
}
#endif