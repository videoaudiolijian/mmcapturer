#include "common_logger.h"

#include <sstream>
#include <new>
#include <thread>

#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>

LogTask::LogTask(const char *filename, int linenumber, const char *msgType, const char *strMessage, int msgsize)
	: m_file_name(filename), m_line_number(linenumber), m_msg_type(msgType), m_message(strMessage, msgsize)
{
}

LogTask::~LogTask()
{
}

AppLogger::AppLogger(const std::string &strPrefixName)
	: SimpleMessageQueue(1024*20),
	m_initialized(false),
	m_is_daily(true),
	m_max_size(4096),
	m_max_file_count(100),
	m_day(0),
	m_seq(0),
	m_file(nullptr),
	m_printed_size(0L),
	m_log_type(LogType::ASYNC)
{
	if (strPrefixName.length() > 128)
	{
		m_prefix_name = strPrefixName.substr(0, 128);
	}
	else
	{
		m_prefix_name = strPrefixName;
	}
}

AppLogger::~AppLogger()
{
}

bool AppLogger::initialize(const std::string &strLogDirectory, bool bDaily, int nMaxSize, int nMaxFile)
{
	if (m_initialized)
	{
		return true;
	}

	m_log_directory = strLogDirectory;
	m_is_daily = bDaily;
	m_max_size = nMaxSize;
	m_max_file_count = nMaxFile;
	m_initialized = true;

	start();
	return true;
}

bool AppLogger::uninitialize()
{
	if (!m_initialized)
	{
		return true;
	}

	m_initialized = false;

	stop();

	if (nullptr != m_file)
	{
		fclose(m_file);
		m_file = nullptr;
		m_printed_size = 0;
	}

	return true;
}

void AppLogger::set_log_type(LogType type)
{
	this->m_log_type = type;
}

void AppLogger::handle_msg(const MessageItem &msg)
{
	std::unique_ptr<LogTask> task(static_cast<LogTask *>(msg.get_data()));

	switch (msg.get_id())
	{
	case MSG_LOG_WRITE_TASK:
	{
		if (nullptr != task)
		{
			write_log(task->m_file_name.c_str(),
				task->m_line_number,
				task->m_msg_type.c_str(),
				task->m_message.c_str());
		}
	}
	break;
	default:
		break;
	}
}

void AppLogger::write_log(const char *filename, int linenumber, const char *msgtype, const char *strMessage)
{
	time_t now = time(NULL);
	struct tm* tmnow = localtime(&now);
	if (nullptr == tmnow)
	{
		return;
	}

	const char *pFileName = filename;

	pFileName = strrchr(filename, '\\');
	if (nullptr != pFileName)
	{
		++pFileName;
	}
	else
	{
		pFileName = strrchr(filename, '/');
		if (nullptr == pFileName)
		{
			pFileName = filename;
		}
		else
		{
			++pFileName;
		}
	}

	char szNow[64] = { 0 };
	char szFileName[256] = { 0 };
	strftime(szNow, sizeof(szNow), "[%y-%m-%d %H:%M:%S]", tmnow);

	if ((m_is_daily && m_day != tmnow->tm_mday) ||
		m_printed_size > (long)(m_max_size * 1024) ||
		m_seq > m_max_file_count ||
		nullptr == m_file)
	{
		if ((m_is_daily && m_day != tmnow->tm_mday) || m_seq > m_max_file_count)
			m_seq = 0;

		bool bLoop = false;
		std::string strLogFile;
		do
		{
			m_seq = m_seq % m_max_file_count + 1;
			memset(szFileName, 0, sizeof(szFileName));
			if (m_is_daily)
			{
				sprintf(szFileName, "/%s-%d-%.2d-%.2d.%d.log", m_prefix_name.c_str(), tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, m_seq);
			}
			else
			{
				sprintf(szFileName, "/%s.%d.log", m_prefix_name.c_str(), m_seq);
			}

			strLogFile = m_log_directory;
			strLogFile += szFileName;
			if (m_log_file_path.empty())
			{
				if (bLoop)
					break;

				FILE *pFile = fopen(strLogFile.c_str(), "rb");
				if (nullptr == pFile)
				{
					break;
				}
				else
				{
					fclose(pFile);
					pFile = nullptr;
				}

				if (m_seq == m_max_file_count)
					bLoop = true;
			}
		} while (m_log_file_path.empty());

		m_log_file_path = strLogFile;
		if (nullptr != m_file)
		{
			fclose(m_file);
			m_file = nullptr;
			m_printed_size = 0;
		}
		m_file = fopen(m_log_file_path.c_str(), "wb");
		if (nullptr != m_file)
		{
			setvbuf(m_file, nullptr, _IONBF, 0);
			m_printed_size = 0;
			m_day = tmnow->tm_mday;
		}
	}

	if (nullptr != m_file)
	{
		std::stringstream oss;
		oss << std::this_thread::get_id();
		std::string stid = oss.str();
		unsigned long long tid = std::stoull(stid);

		int size = fprintf(m_file, "%s%s[%llu] %s(%d):\t\t%s\r\n", szNow, msgtype, tid, pFileName, linenumber, strMessage);
		fflush(m_file);
		if (size > 0)
		{
			m_printed_size += size;
		}
	}
}

void AppLogger::info(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[I]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[I]", szLog, nLog));
			put(msg);
		}
	}
}

void AppLogger::debug(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[D]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[D]", szLog, nLog));
			put(msg);
		}
	}
}

void AppLogger::warning(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[W]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[W]", szLog, nLog));
			put(msg);
		}
	}
}

void AppLogger::error(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[E]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[E]", szLog, nLog));
			put(msg);
		}
	}
}

void AppLogger::trace(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[T]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[T]", szLog, nLog));
			put(msg);
		}
	}
}

void AppLogger::verbose(const char *filename, int line, const char *szFormat, ...)
{
	char szLog[4096];

	va_list a_list;
	memset(szLog, 0, sizeof(szLog));
	va_start(a_list, szFormat);
	int nLog = vsnprintf(szLog, 4000, szFormat, a_list);
	va_end(a_list);

	if (nLog > 0)
	{
		if (m_log_type == LogType::SYNC)
		{
			std::lock_guard<std::mutex> lg(m_log_mutex);
			write_log(filename, line, "[V]", szLog);
		}
		else
		{
			MessageItem msg(MSG_LOG_WRITE_TASK, 0, 0, new (std::nothrow) LogTask(filename, line, "[V]", szLog, nLog));
			put(msg);
		}
	}
}