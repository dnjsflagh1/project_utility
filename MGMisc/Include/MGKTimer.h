/********************************************************************************
*				@多核cpu上 使用本类可能造成的结果不正确原因：					*	
*	1、不能保证同一块主板上每个核的 TSC 是同步的								*
*	2、CPU 的时钟频率可能变化，如笔记本电脑的节能功能							*
*	3、乱序执行导致 RDTSC 测得的周期数不准.										*
*				@解决办法：														*
*	1、获取cpu时钟时前先将线程锁定到特定的核									*
********************************************************************************/
#ifndef _MGKTIMER_H_
#define _MGKTIMER_H_
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	inline U64 GetCycleCount(void)
	{
		//_asm RDTSC 早期版本不支持。。。换成下面的
		_asm _emit 0x0F; 
		_asm _emit 0x31;
	}
	/******************************************************************************/
	class MGKTimer
	{
	public:
		MGKTimer(void);
		// 启动CPU时钟
				void	start(void);
		// 停止CPU时钟, 返回自上一次启动的时钟周期数
				U64		stop(void);
		// 把以CPU周期数转为纳秒
		static	U64		cyclesToNanos(U64 timeCycles, UInt speedMhz);
		// 把以CPU周期数转为毫秒
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