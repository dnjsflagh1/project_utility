#ifndef _LOGCONSOLE_H_
#define _LOGCONSOLE_H_
namespace MG
{
	enum LOG_TYPE
	{
		out_sys			=	BIT(0),
		out_netsend		=	BIT(1),
		out_netrecv		=	BIT(2),
		out_error		=	BIT(3),
		out_debug		=	BIT(4),
		out_warning		=	BIT(5),
		out_info		=	BIT(6),
		out_script_dbg	=	BIT(7),
		out_game_logic	=	BIT(8),
		out_trace		=	BIT(9),
		out_max         =	BIT(10),
	};

	class LogConsole
	{
	public:
		LogConsole();
		virtual ~LogConsole();
		Bool		openConsole( );
		Bool		isOpen() const;
		void		closeConsole();
		void		logWrite( LOG_TYPE outType, CChar* logStr );
	};
}
#endif
