#ifndef VARIABLE_COLLECTION_H
#define VARIABLE_COLLECTION_H

#include "Utilities/Utilities.h"
#if USE_QT
#include <QString.h>
#include <QVector.h>
#include <QFile.h>
#include <QTextStream.h>
#endif // USE_QT
#if USE_STL
#include <string.h>
#include <vector>
#include <fstream>
#endif // USE_STL
#include "Variable System/Variable.h"

class VariableCollection {
public:
	VariableCollection();
	VariableCollection(const VariableCollection & v);
	VariableCollection & operator = (const VariableCollection & v);
	virtual ~VariableCollection();
	
	int numberOfCategories() const;
	int indexOfCategory(const char * category) const;
#if USE_QT
	int indexOfCategory(const QString & category) const;
#endif // USE_QT
#if USE_STL
	int indexOfCategory(const std::string & category) const;
#endif // USE_STL
	char * getCategory(int index) const;
	int addCategory(const char * category);
#if USE_QT
	int addCategory(const QString & category);
#endif // USE_QT
#if USE_STL
	int addCategory(const std::string & category);
#endif // USE_STL
	bool removeCategory(const char * data);
#if USE_QT
	bool removeCategory(const QString & data);
#endif // USE_QT
#if USE_STL
	bool removeCategory(const std::string & data);
#endif // USE_STL
	int numberOfVariables() const;
	bool hasVariable(const char * id) const;
#if USE_QT
	bool hasVariable(const QString & id) const;
#endif // USE_QT
#if USE_STL
	bool hasVariable(const std::string & id) const;
#endif // USE_STL
	bool hasVariable(const char * id, const char * category) const;
#if USE_QT
	bool hasVariable(const QString & id, const QString & category) const;
#endif // USE_QT
#if USE_STL
	bool hasVariable(const std::string & id, const std::string & category) const;
#endif // USE_STL
	bool hasVariable(const Variable * v) const;
	int indexOfVariable(const char * id) const;
#if USE_QT
	int indexOfVariable(const QString & id) const;
#endif // USE_QT
#if USE_STL
	int indexOfVariable(const std::string & id) const;
#endif // USE_STL
	int indexOfVariable(const char * id, const char * category) const;
#if USE_QT
	int indexOfVariable(const QString & id, const QString & category) const;
#endif // USE_QT
#if USE_STL
	int indexOfVariable(const std::string & id, const std::string & category) const;
#endif // USE_STL
	int indexOfVariable(const Variable * v) const;
	const Variable * getVariable(int index) const;
	const Variable * getVariable(const char * id) const;
#if USE_QT
	const Variable * getVariable(const QString & id) const;
#endif // USE_QT
#if USE_STL
	const Variable * getVariable(const std::string & id) const;
#endif // USE_STL
	const Variable * getVariable(const char * id, const char * category) const;
#if USE_QT
	const Variable * getVariable(const QString & id, const QString & category) const;
#endif // USE_QT
#if USE_STL
	const Variable * getVariable(const std::string & id, const std::string & category) const;
#endif // USE_STL
	const char * getValue(const char * id) const;
#if USE_QT
	const char * getValue(const QString & id) const;
#endif // USE_QT
#if USE_STL
	const char * getValue(const std::string & id) const;
#endif // USE_STL
	const char * getValue(const char * id, const char * category) const;
#if USE_QT
	const char * getValue(const QString & id, const QString & category) const;
#endif // USE_QT
#if USE_STL
	const char * getValue(const std::string & id, const std::string & category) const;
#endif // USE_STL
#if USE_QT >= USE_STL
	QVector<Variable *> * getVariablesInCategory(const char * category) const;
	QVector<Variable *> * getVariablesInCategory(const QString & category) const;
#elif USE_STL
	std::vector<Variable *> * getVariablesInCategory(const char * category) const;
	std::vector<Variable *> * getVariablesInCategory(const std::string & category) const;
#endif // USE_QT
	void setValue(const char * id, const char * value);
	void setValue(const char * id, int value);
	void setValue(const char * id, double value);
	void setValue(const char * id, bool value);
#if USE_QT
	void setValue(const QString & id, const char * value);
	void setValue(const QString & id, const QString & value);
	void setValue(const QString & id, int value);
	void setValue(const QString & id, double value);
	void setValue(const QString & id, bool value);
#endif // USE_QT
#if USE_STL
	void setValue(const std::string & id, const char * value);
	void setValue(const std::string & id, const std::string & value);
	void setValue(const std::string & id, int value);
	void setValue(const std::string & id, double value);
	void setValue(const std::string & id, bool value);
#endif // USE_STL
	void setValue(const char * id, const char * value, const char * category);
	void setValue(const char * id, int value, const char * category);
	void setValue(const char * id, double value, const char * category);
	void setValue(const char * id, bool value, const char * category);
#if USE_QT
	void setValue(const QString & id, const char * value, const QString & category);
	void setValue(const QString & id, const QString & value, const QString & category);
	void setValue(const QString & id, int value, const QString & category);
	void setValue(const QString & id, double value, const QString & category);
	void setValue(const QString & id, bool value, const QString & category);
#endif // USE_QT
#if USE_STL
	void setValue(const std::string & id, const char * value, const std::string & category);
	void setValue(const std::string & id, const std::string & value, const std::string & category);
	void setValue(const std::string & id, int value, const std::string & category);
	void setValue(const std::string & id, double value, const std::string & category);
	void setValue(const std::string & id, bool value, const std::string & category);
#endif // USE_STL
	bool addVariable(const char * id, const char * value = NULL, const char * category = NULL);
#if USE_QT
	bool addVariable(const QString & id, const QString & value = QString(), const QString & category = QString());
#endif // USE_QT
#if USE_STL
	bool addVariable(const std::string & id, const std::string & value = std::string(), const std::string & category = std::string());
#endif // USE_STL
	bool addVariable(Variable * v);
	bool removeVariable(int index);
	bool removeVariable(const char * id);
#if USE_QT
	bool removeVariable(const QString & id);
#endif // USE_QT
#if USE_STL
	bool removeVariable(const std::string & id);
#endif // USE_STL
	bool removeVariable(const char * id, const char * category);
#if USE_QT
	bool removeVariable(const QString & id, const QString & category);
#endif // USE_QT
#if USE_STL
	bool removeVariable(const std::string & id, const std::string & category);
#endif // USE_STL
	bool removeVariable(const Variable * v);
	void clearVariables();
	void sortVariables();

	static VariableCollection * readFrom(const char * fileName);
#if USE_QT
	static VariableCollection * readFrom(const QString & fileName);
#endif // USE_QT
#if USE_STL
	static VariableCollection * readFrom(const std::string & fileName);
#endif // USE_STL
	bool writeTo(const char * fileName) const;
#if USE_QT
	bool writeTo(const QString & fileName) const;
#endif // USE_QT
#if USE_STL
	bool writeTo(const std::string & fileName) const;
#endif // USE_STL

	bool operator == (const VariableCollection & v) const;
	bool operator != (const VariableCollection & v) const;

private:
#if USE_QT >= USE_STL
	static QVector<Variable *> mergeSort(QVector<Variable *> variables);
	static QVector<Variable *> merge(QVector<Variable *> left, QVector<Variable *> right);
#elif USE_STL
	static std::vector<Variable *> mergeSort(std::vector<Variable *> variables);
	static std::vector<Variable *> merge(std::vector<Variable *> left, std::vector<Variable *> right);
#endif // USE_QT

private:
#if USE_QT >= USE_STL
	QVector<Variable *> m_variables;
	QVector<char *> m_categories;
#elif USE_STL
	std::vector<Variable *> m_variables;
	std::vector<char *> m_categories;
#endif // USE_STL
};

#endif // VARIABLE_COLLECTION_H
