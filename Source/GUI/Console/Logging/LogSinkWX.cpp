#include "LogSinkWX.h"

#include "../../WXUtilities.h"

#include <Logging/LogSystem.h>

#include <iostream>

#include <windows.h>

LogSinkWX::LogSinkWX()
	: m_initialized(false) {
	set_pattern("%^%T.%e [%t] %L: %v%$");
}

LogSinkWX::~LogSinkWX() {
	m_logLevelChangedConnection.disconnect();
}

void LogSinkWX::initialize() {
	if(m_initialized) {
		return;
	}

	LogSystem * logSystem = LogSystem::getInstance();
	m_logLevelChangedConnection = logSystem->logLevelChanged.connect(std::bind(&LogSinkWX::onLogLevelChanged, this, std::placeholders::_1));

	wxLog::SetVerbose(true);
	wxLog::SetLogLevel(WXUtilities::spdLogLevelToWXLogLevel(logSystem->getLevel()));

	for(const std::pair<wxLogLevel, std::string> & logMessage : m_logMessageCache) {
		wxLogGeneric(logMessage.first, "%s", logMessage.second.data());
	}

	m_logMessageCache.clear();

	m_initialized = true;
}

void LogSinkWX::sink_it_(const spdlog::details::log_msg & logMessage) {
	m_formatBuffer.clear();
	formatter_->format(logMessage, m_formatBuffer);
	m_formatBuffer.push_back('\0');

	wxLogLevel logLevel = WXUtilities::spdLogLevelToWXLogLevel(logMessage.level);
	std::string_view formattedLogMessageWithNewlines(m_formatBuffer.data(), m_formatBuffer.size());
	std::string formattedLogMessageWithoutNewlines(formattedLogMessageWithNewlines.data(), formattedLogMessageWithNewlines.find_first_of("\r\n"));

	if(m_initialized) {
		wxLogGeneric(logLevel, "%s", formattedLogMessageWithoutNewlines.data());
	}
	else {
		m_logMessageCache.push_back(std::make_pair(logLevel, formattedLogMessageWithoutNewlines));
	}
}

void LogSinkWX::flush_() { }

void LogSinkWX::onLogLevelChanged(spdlog::level::level_enum logLevel) {
	wxLogLevel newLogLevel = WXUtilities::spdLogLevelToWXLogLevel(logLevel);

	spdlog::debug("Updating wxWidgets log level from '{}' to '{}'.", WXUtilities::logLevelToString(wxLog::GetLogLevel()), WXUtilities::logLevelToString(newLogLevel));

	wxLog::SetLogLevel(newLogLevel);
}
