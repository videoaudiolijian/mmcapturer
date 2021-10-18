#ifndef _H_COMMON_LOGGER_H_
#define _H_COMMON_LOGGER_H_

#include <stdio.h>
#include <string>
#include <iostream>
#include <mutex>

#include "common_msg_queue.h"

#define LOG_LEVEL_NONE     0x00000000	 /* none    */
#define LOG_LEVEL_ERROR    0x00000001	 /* error   */
#define LOG_LEVEL_WARNING  0x00000002    /* warning */
#define LOG_LEVEL_INFO     0x00000004	 /* info    */
#define LOG_LEVEL_DEBUG    0x00000008	 /* debug   */
#define LOG_LEVEL_TRACE    0x00000010	 /* trace   */
#define LOG_LEVEL_VERBOSE  0x00000020    /* verbose */

class AppLogger;
extern AppLogger *g_pLogger;

extern int g_log_level;

#define LOG_INFO(x, ...)                                             \
	do                                                               \
	{                                                                \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_INFO))            \
			g_pLogger->info(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)
#define LOG_WARNING(x, ...)                                             \
	do                                                                  \
	{                                                                   \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_WARNING))            \
			g_pLogger->warning(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)
#define LOG_ERROR(x, ...)                                             \
	do                                                                \
	{                                                                 \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_ERROR))            \
			g_pLogger->error(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)
#define LOG_DEBUG(x, ...)                                             \
	do                                                                \
	{                                                                 \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_DEBUG))            \
			g_pLogger->debug(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)
#define LOG_TRACE(x, ...)                                             \
	do                                                                \
	{                                                                 \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_TRACE))            \
			g_pLogger->trace(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)
#define LOG_VERBOSE(x, ...)                                             \
	do                                                                  \
	{                                                                   \
		if (g_pLogger && (g_log_level >= LOG_LEVEL_VERBOSE))            \
			g_pLogger->verbose(__FILE__, __LINE__, (x), ##__VA_ARGS__); \
	} while (0)


constexpr int  MSG_LOG_WRITE_TASK = 1001;

class LogTask
{
public:
	LogTask(const char *filename, int linenumber, const char *msgType, const char *strMessage, int msgSize);
	virtual ~LogTask();

public:
	std::string m_file_name;
	int m_line_number;
	std::string m_msg_type;
	std::string m_message;
};

//log type: synchronize asynchronize
enum class LogType : int
{
	SYNC,
	ASYNC
};

class AppLogger : public SimpleMessageQueue
{
public:
	AppLogger() = delete;
	AppLogger(const AppLogger &) = delete;
	AppLogger &operator=(const AppLogger &) = delete;
	AppLogger(const AppLogger &&) = delete;
	AppLogger &operator=(const AppLogger &&) = delete;

	//constructor
	//@param£ºstrPrefixName -- file name prefix
	AppLogger(const std::string &strPrefixName);
	virtual ~AppLogger();

	//set the log type
	void set_log_type(LogType type);

	//initialize
	//@param£ºstrLogPath -- file directory
	//      bDaily -- whether file name contains daily date info
	//      nMaxSize -- file max size, in kb
	//      nMaxFile -- max file count
	bool initialize(const std::string &strLogDirectory, bool bDaily, int nMaxSize, int nMaxFile);
	bool uninitialize();

	void info(const char *filename, int line, const char *szFormat, ...);
	void debug(const char *filename, int line, const char *szFormat, ...);
	void warning(const char *filename, int line, const char *szFormat, ...);
	void error(const char *filename, int line, const char *szFormat, ...);
	void trace(const char *filename, int line, const char *szFormat, ...);
	void verbose(const char *filename, int line, const char *szFormat, ...);

protected:
	virtual void handle_msg(const MessageItem &msg);
	virtual void destroy_msg(const MessageItem &msg)
	{
		std::unique_ptr<LogTask> task(static_cast<LogTask *>(msg.get_data()));
	}

private:
	//write log
	//@param£º
	//      filename -- source file name
	//       linenumber -- code line number
	//       msgType -- msg type£¬[E],[W],[I],[D],[V], are error, warning, info, debug, verbose
	//       strMessage -- string to write to log
	void write_log(const char *filename, int linenumber, const char *msgType, const char *strMessage);

private:
	bool m_initialized;

	//log file prefix
	std::string m_prefix_name;
	//log file directory
	std::string m_log_directory;

	//if the log file name contains daily date info
	bool m_is_daily;

	//file size, in kb
	int m_max_size;
	//file count
	int m_max_file_count;

	//1 to 31
	int m_day;

	//current file sequence
	int m_seq;

	//file, file path and file name
	std::string m_log_file_path;

	//file
	FILE * m_file;

	//log type
	LogType m_log_type;

	//the printed log size
	long m_printed_size;

	std::mutex m_log_mutex;
};

#endif