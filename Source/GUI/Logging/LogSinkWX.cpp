#include "LogSinkWX.h"

#include <Logging/LogSystem.h>

#include <iostream>

#include <windows.h>

LogSinkWX::LogSinkWX()
	: m_initialized(false) {
	set_pattern("%^%T.%e [%t] %L: %v%$");
}

LogSinkWX::~LogSinkWX() { }

void LogSinkWX::initialize() {
	if(m_initialized) {
		return;
	}

	for(const std::pair<wxLogLevel, std::string> & logMessage : m_logMessageCache) {
		wxLogGeneric(logMessage.first, logMessage.second.data());
	}

	m_logMessageCache.clear();

	m_initialized = true;
}

void LogSinkWX::sink_it_(const spdlog::details::log_msg & logMessage) {
	m_formatBuffer.clear();
	formatter_->format(logMessage, m_formatBuffer);
	m_formatBuffer.push_back('\0');

	wxLogLevel logLevel = wxLOG_Message;

	switch(logMessage.level) {
		case spdlog::level::level_enum::trace: {
			logLevel = wxLOG_Trace;
			break;
		}

		case spdlog::level::level_enum::debug: {
			logLevel = wxLOG_Debug;
			break;
		}

		case spdlog::level::level_enum::info: {
			logLevel = wxLOG_Info;
			break;
		}

		case spdlog::level::level_enum::warn: {
			logLevel = wxLOG_Warning;
			break;
		}

		case spdlog::level::level_enum::err: {
			logLevel = wxLOG_Error;
			break;
		}

		case spdlog::level::level_enum::critical: {
			logLevel = wxLOG_FatalError;
			break;
		}

		case spdlog::level::level_enum::off:
		case spdlog::level::level_enum::n_levels: {
			break;
		}
	}

	std::string_view formattedLogMessageWithNewlines(m_formatBuffer.data(), m_formatBuffer.size());
	std::string formattedLogMessageWithoutNewlines(formattedLogMessageWithNewlines.data(), formattedLogMessageWithNewlines.find_first_of("\r\n"));

	if(m_initialized) {
		wxLogGeneric(logLevel, formattedLogMessageWithoutNewlines.data());
	}
	else {
		m_logMessageCache.push_back(std::make_pair(logLevel, formattedLogMessageWithoutNewlines));
	}
}

void LogSinkWX::flush_() { }
