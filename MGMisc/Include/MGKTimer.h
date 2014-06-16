/********************************************************************************
*				@���cpu�� ʹ�ñ��������ɵĽ������ȷԭ��					*	
*	1�����ܱ�֤ͬһ��������ÿ���˵� TSC ��ͬ����								*
*	2��CPU ��ʱ��Ƶ�ʿ��ܱ仯����ʼǱ����ԵĽ��ܹ���							*
*	3������ִ�е��� RDTSC ��õ���������׼.										*
*				@����취��														*
*	1����ȡcpuʱ��ʱǰ�Ƚ��߳��������ض��ĺ�									*
********************************************************************************/
#ifndef _MGKTIMER_H_
#define _MGKTIMER_H_
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	inline U64 GetCycleCount(void)
	{
		//_asm RDTSC ���ڰ汾��֧�֡��������������
		_asm _emit 0x0F; 
		_asm _emit 0x31;
	}
	/******************************************************************************/
	class MGKTimer
	{
	public:
		MGKTimer(void);
		// ����CPUʱ��
				void	start(void);
		// ֹͣCPUʱ��, ��������һ��������ʱ��������
				U64		stop(void);
		// ����CPU������תΪ����
		static	U64		cyclesToNanos(U64 timeCycles, UInt speedMhz);
		// ����CPU������תΪ����
		static	U64		cyclesToMillis(U64 timeCycles,UInt speedMhz);
		// 1GHz = 1000MHz
		static	UInt	cpuSpeedMHz();

		static	UInt	getCpuMHz();	
	private:
		U64				mStartcycle;
		U64				mOverhead;// Clock Cycles
		static	UInt	mCpuMHz;
	};
	/******************************************************************************/
}


#endif //_MGKTIMER_H_