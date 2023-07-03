#ifndef _LOG_SINK_WINDOWS_H_
#define _LOG_SINK_WINDOWS_H_

#include <boost/signals2.hpp>
#include <fmt/format.h>
#include <spdlog/sinks/base_sink.h>
#include <wx/log.h>

#include <mutex>

class LogSinkWX : public spdlog::sinks::base_sink<std::mutex> {
public:
	LogSinkWX();
	virtual ~LogSinkWX();

	void initialize();

protected:
	virtual void sink_it_(const spdlog::details::log_msg & logMessage) override;
	virtual void flush_() override;

private:
	void onLogLevelChanged(spdlog::level::level_enum logLevel);

	bool m_initialized;
	std::vector<std::pair<wxLogLevel, std::string>> m_logMessageCache;
	spdlog::memory_buf_t m_formatBuffer;
	boost::signals2::connection m_logLevelChangedConnection;

	LogSinkWX(const LogSinkWX &) = delete;
	const LogSinkWX & operator = (const LogSinkWX &) = delete;
};

#endif // _LOG_SINK_WINDOWS_H_
