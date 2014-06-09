#ifndef MOD_SCREENSHOT_H
#define MOD_SCREENSHOT_H

#include <QString.h>
#include "Utilities/Utilities.h"

class ModScreenshot {
public:
	ModScreenshot(const char * fileName, const char * thumbnail = NULL, const char * width = NULL, const char * height = NULL);
	ModScreenshot(const char * fileName, const char * thumbnail, int width, int height);
	ModScreenshot(const QString & fileName, const QString & thumbnail = QString(), const QString & width = QString(), const QString & height = QString());
	ModScreenshot(const QString & fileName, const QString & thumbnail, int width, int height);
	ModScreenshot(const ModScreenshot & s);
	ModScreenshot & operator = (const ModScreenshot & s);
	~ModScreenshot();

	const char * getFileName() const;
	const char * getThumbnail() const;
	int getWidth() const;
	int getHeight() const;

	void setFileName(const char * fileName);
	void setFileName(const QString & fileName);
	void setThumbnail(const char * thumbnail);
	void setThumbnail(const QString & thumbnail);
	void setWidth(int width);
	void setWidth(const char * width);
	void setWidth(const QString & width);
	void setHeight(int height);
	void setHeight(const char * height);
	void setHeight(const QString & height);

	bool operator == (const ModScreenshot & s) const;
	bool operator != (const ModScreenshot & s) const;

private:
	char * m_fileName;
	char * m_thumbnail;
	int m_width;
	int m_height;
};

#endif // MOD_SCREENSHOT_H