#include "Variable System/VariableCollection.h"

VariableCollection::VariableCollection() {

}

VariableCollection::VariableCollection(const VariableCollection & v) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	for(i=0;i<v.m_categories.size();i++) {
		m_categories.push_back(Utilities::trimCopyString(v.m_categories[i]));
	}

	for(i=0;i<v.m_variables.size();i++) {
		m_variables.push_back(new Variable(*(v.m_variables[i])));
	}
}

VariableCollection & VariableCollection::operator = (const VariableCollection & v) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	for(i=0;i<m_categories.size();i++) {
		delete [] m_categories[i];
	}
	m_categories.clear();

	for(i=0;i<m_variables.size();i++) {
		delete m_variables[i];
	}
	m_variables.clear();

	for(i=0;i<v.m_categories.size();i++) {
		m_categories.push_back(Utilities::trimCopyString(v.m_categories[i]));
	}

	for(i=0;i<v.m_variables.size();i++) {
		m_variables.push_back(new Variable(*(v.m_variables[i])));
	}

	return *this;
}

VariableCollection::~VariableCollection() {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	for(i=0;i<m_categories.size();i++) {
		delete [] m_categories[i];
	}

	for(i=0;i<m_variables.size();i++) {
		delete m_variables[i];
	}
}

int VariableCollection::numberOfCategories() const {
#if USE_STL > USE_QT
	return static_cast<int>(m_categories.size());
#else
	return m_categories.size();
#endif // USE_STL
}

int VariableCollection::indexOfCategory(const char * category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category == NULL || Utilities::stringLength(category) == 0 || m_categories.size() == 0) { return -1; }
	
	char * formattedCategory = Utilities::trimCopyString(category);

	if(Utilities::stringLength(formattedCategory) == 0) {
		delete [] formattedCategory;
		return -1;
	}

	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategory, m_categories[i]) == 0) {
			delete [] formattedCategory;
			return i;
		}
	}
	
	delete [] formattedCategory;

	return -1;
}

#if USE_QT
int VariableCollection::indexOfCategory(const QString & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.isEmpty()) { return -1; }

	QString formattedCategory = category.trimmed();
	
	if(formattedCategory.isEmpty()) { return -1; }

	QByteArray formattedCategoryBytes = formattedCategory.toLocal8Bit();

	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategoryBytes.data(), m_categories[i]) == 0) {
			return i;
		}
	}
	
	return -1;
}
#endif // USE_QT

#if USE_STL
int VariableCollection::indexOfCategory(const std::string & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.empty()) { return -1; }

	std::string formattedCategory = Utilities::trimString(category);
	
	if(formattedCategory.empty()) { return -1; }

	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategory.data(), m_categories[i]) == 0) {
			return i;
		}
	}
	
	return -1;
}
#endif // USE_STL

char * VariableCollection::getCategory(int index) const {
#if USE_STL > USE_QT
	if(index < 0 || index >= static_cast<int>(m_categories.size())) { return NULL; }
#else
	if(index < 0 || index >= m_categories.size()) { return NULL; }
#endif // USE_STL

	return m_categories[index];
}

int VariableCollection::addCategory(const char * category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category == NULL || Utilities::stringLength(category) == 0) { return Variable::NO_CATEGORY; }
	
	char * formattedCategory = Utilities::trimCopyString(category);

	if(Utilities::stringLength(formattedCategory) == 0) {
		delete [] formattedCategory;
		return Variable::NO_CATEGORY;
	}

	int categoryIndex = -1;
	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategory, m_categories[i]) == 0) {
			categoryIndex = i;
			break;
		}
	}

	if(categoryIndex == -1) {
		m_categories.push_back(formattedCategory);

#if USE_STL > USE_QT
		return static_cast<int>(m_categories.size()) - 1;
#else
		return m_categories.size() - 1;
#endif // USE_STL
	}
	else {
		delete [] formattedCategory;

		return categoryIndex;
	}
}

#if USE_QT
int VariableCollection::addCategory(const QString & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.isEmpty()) { return Variable::NO_CATEGORY; }
	
	QString formattedCategory = category.trimmed();

	if(formattedCategory.isEmpty()) { return Variable::NO_CATEGORY; }

	QByteArray formattedCategoryBytes = formattedCategory.toLocal8Bit();

	int categoryIndex = -1;
	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategoryBytes.data(), m_categories[i]) == 0) {
			categoryIndex = i;
			break;
		}
	}

	if(categoryIndex == -1) {
		m_categories.push_back(Utilities::copyString(formattedCategoryBytes.data()));

#if USE_STL > USE_QT
		return static_cast<int>(m_categories.size()) - 1;
#else
		return m_categories.size() - 1;
#endif // USE_STL
	}
	else {
		return categoryIndex;
	}
}
#endif // USE_QT

#if USE_STL
int VariableCollection::addCategory(const std::string & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.empty()) { return Variable::NO_CATEGORY; }
	
	std::string formattedCategory = Utilities::trimString(category);

	if(formattedCategory.empty()) { return Variable::NO_CATEGORY; }

	int categoryIndex = -1;
	for(i=0;i<m_categories.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedCategory.data(), m_categories[i]) == 0) {
			categoryIndex = i;
			break;
		}
	}

	if(categoryIndex == -1) {
		m_categories.push_back(Utilities::copyString(formattedCategory.data()));

#if USE_STL > USE_QT
		return static_cast<int>(m_categories.size()) - 1;
#else
		return m_categories.size() - 1;
#endif // USE_STL
	}
	else {
		return categoryIndex;
	}
}
#endif // USE_STL

bool VariableCollection::removeCategory(const char * category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category == NULL || Utilities::stringLength(category) == 0) { return false; }

	char * formattedCategory = Utilities::trimCopyString(category);

	if(Utilities::stringLength(category) == 0) {
		delete [] formattedCategory;
		return false;
	}

	int categoryIndex = indexOfCategory(formattedCategory);

	if(categoryIndex == -1) {
		delete [] formattedCategory;
		return false;
	}

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			i--;
		}
		else if(m_variables[i]->getCategory() > categoryIndex) {
			m_variables[i]->setCategory(m_variables[i]->getCategory() - 1);
		}
	}

	delete [] formattedCategory;

#if USE_STL > USE_QT
	if(categoryIndex >= 0 && categoryIndex < static_cast<int>(m_categories.size())) {
#else
	if(categoryIndex >= 0 && categoryIndex < m_categories.size()) {
#endif // USE_STL
		delete [] m_categories[categoryIndex];
#if USE_STL > USE_QT
		m_categories.erase(m_categories.begin() + categoryIndex);
#else
		m_categories.remove(categoryIndex);
#endif // USE_STL
	}

	return true;
}

#if USE_QT
bool VariableCollection::removeCategory(const QString & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.isEmpty()) { return false; }

	QString formattedCategory = category.trimmed();

	if(formattedCategory.isEmpty()) { return false; }

	int categoryIndex = indexOfCategory(formattedCategory);

	if(categoryIndex == -1) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			i--;
		}
		else if(m_variables[i]->getCategory() > categoryIndex) {
			m_variables[i]->setCategory(m_variables[i]->getCategory() - 1);
		}
	}
	
#if USE_STL > USE_QT
	if(categoryIndex >= 0 && categoryIndex < static_cast<int>(m_categories.size())) {
#else
	if(categoryIndex >= 0 && categoryIndex < m_categories.size()) {
#endif // USE_STL
		delete [] m_categories[categoryIndex];
#if USE_STL > USE_QT
		m_categories.erase(m_categories.begin() + categoryIndex);
#else
		m_categories.remove(categoryIndex);
#endif // USE_STL
	}

	return true;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::removeCategory(const std::string & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(category.empty()) { return false; }

	std::string formattedCategory = Utilities::trimString(category);

	if(category.empty()) { return false; }

	int categoryIndex = indexOfCategory(formattedCategory);

	if(categoryIndex == -1) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			i--;
		}
		else if(m_variables[i]->getCategory() > categoryIndex) {
			m_variables[i]->setCategory(m_variables[i]->getCategory() - 1);
		}
	}

#if USE_STL > USE_QT
	if(categoryIndex >= 0 && categoryIndex < static_cast<int>(m_categories.size())) {
#else
	if(categoryIndex >= 0 && categoryIndex < m_categories.size()) {
#endif // USE_STL
		delete [] m_categories[categoryIndex];
#if USE_STL > USE_QT
		m_categories.erase(m_categories.begin() + categoryIndex);
#else
		m_categories.remove(categoryIndex);
#endif // USE_STL
	}

	return true;
}
#endif // USE_STL

int VariableCollection::numberOfVariables() const {
#if USE_STL > USE_QT
	return static_cast<int>(m_variables.size());
#else
	return m_variables.size();
#endif // USE_STL
}

bool VariableCollection::hasVariable(const char * id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return false; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return false;
	}

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return true;
		}
	}

	delete [] formattedID;

	return false;
}

#if USE_QT
bool VariableCollection::hasVariable(const QString & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return false; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return false; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return true;
		}
	}

	return false;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::hasVariable(const std::string & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return false; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return true;
		}
	}

	return false;
}
#endif // USE_STL

bool VariableCollection::hasVariable(const char * id, const char * category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return false; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return false;
	}

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return true;
		}
	}

	delete [] formattedID;

	return false;
}

#if USE_QT
bool VariableCollection::hasVariable(const QString & id, const QString & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return false; }

	QString formattedID = id.trimmed();
	
	if(formattedID.isEmpty()) { return false; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return true;
		}
	}

	return false;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::hasVariable(const std::string & id, const std::string & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return false; }

	std::string formattedID = Utilities::trimString(id);
	
	if(formattedID.empty()) { return false; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return true;
		}
	}

	return false;
}
#endif // USE_STL

bool VariableCollection::hasVariable(const Variable * v) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(v == NULL) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(v->getID(), m_variables[i]->getID()) == 0 &&
			v->getCategory() == m_variables[i]->getCategory()) {
			return true;
		}
	}

	return false;
}

int VariableCollection::indexOfVariable(const char * id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return -1; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return -1;
	}

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return i;
		}
	}

	delete [] formattedID;

	return -1;
}

#if USE_QT
int VariableCollection::indexOfVariable(const QString & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return -1; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return -1; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return i;
		}
	}

	return -1;
}
#endif // USE_QT

#if USE_STL
int VariableCollection::indexOfVariable(const std::string & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return -1; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return -1; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return i;
		}
	}

	return -1;
}
#endif // USE_STL

int VariableCollection::indexOfVariable(const char * id, const char * category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return -1; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return -1;
	}
	
	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return i;
		}
	}

	delete [] formattedID;

	return -1;
}

#if USE_QT
int VariableCollection::indexOfVariable(const QString & id, const QString & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return -1; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return -1; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return i;
		}
	}

	return -1;
}
#endif // USE_QT

#if USE_STL
int VariableCollection::indexOfVariable(const std::string & id, const std::string & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return -1; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return -1; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return i;
		}
	}

	return -1;
}
#endif // USE_STL

int VariableCollection::indexOfVariable(const Variable * v) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(v == NULL) { return -1; }

	for(i=0;i<m_variables.size();i++) {
		if(v->getCategory() == m_variables[i]->getCategory() &&
		   Utilities::compareStringsIgnoreCase(v->getID(), m_variables[i]->getID()) == 0) {
			return i;
		}
	}

	return -1;
}

const Variable * VariableCollection::getVariable(int index) const {
#if USE_STL > USE_QT
	if(index < 0 || index >= static_cast<int>(m_variables.size())) { return NULL; }
#else
	if(index < 0 || index >= m_variables.size()) { return NULL; }
#endif // USE_STL

	return m_variables[index];
}

const Variable * VariableCollection::getVariable(const char * id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return NULL; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return NULL;
	}

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return m_variables[i];
		}
	}

	delete [] formattedID;

	return NULL;
}

#if USE_QT
const Variable * VariableCollection::getVariable(const QString & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return NULL; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return NULL; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i];
		}
	}

	return NULL;
}
#endif // USE_QT

#if USE_STL
const Variable * VariableCollection::getVariable(const std::string & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return NULL; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return NULL; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i];
		}
	}

	return NULL;
}
#endif // USE_STL

const Variable * VariableCollection::getVariable(const char * id, const char * category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return NULL; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return NULL;
	}

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return m_variables[i];
		}
	}

	delete [] formattedID;

	return NULL;
}

#if USE_QT
const Variable * VariableCollection::getVariable(const QString & id, const QString & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return NULL; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return NULL; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i];
		}
	}

	return NULL;
}
#endif // USE_QT

#if USE_STL
const Variable * VariableCollection::getVariable(const std::string & id, const std::string & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return NULL; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return NULL; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i];
		}
	}

	return NULL;
}
#endif // USE_STL

const char * VariableCollection::getValue(const char * id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return NULL; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return NULL;
	}

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return m_variables[i]->getValue();
		}
	}

	delete [] formattedID;

	return NULL;
}

#if USE_QT
const char * VariableCollection::getValue(const QString & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return NULL; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return NULL; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i]->getValue();
		}
	}

	return NULL;
}
#endif // USE_QT

#if USE_STL
const char * VariableCollection::getValue(const std::string & id) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return NULL; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return NULL; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i]->getValue();
		}
	}

	return NULL;
}
#endif // USE_STL

const char * VariableCollection::getValue(const char * id, const char * category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return NULL; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return NULL;
	}

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete [] formattedID;
			return m_variables[i]->getValue();
		}
	}

	delete [] formattedID;

	return NULL;
}

#if USE_QT
const char * VariableCollection::getValue(const QString & id, const QString & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return NULL; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return NULL; }

	int categoryIndex = indexOfCategory(category);

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i]->getValue();
		}
	}

	return NULL;
}
#endif // USE_QT

#if USE_STL
const char * VariableCollection::getValue(const std::string & id, const std::string & category) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return NULL; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return NULL; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			return m_variables[i]->getValue();
		}
	}

	return NULL;
}
#endif // USE_STL

#if USE_QT >= USE_STL
QVector<Variable *> * VariableCollection::getVariablesInCategory(const char * category) const {
	int categoryIndex = indexOfCategory(category);

	QVector<Variable *> * variableCollection = new QVector<Variable * >;
	
	for(int i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex) {
			variableCollection->push_back(m_variables[i]);
		}
	}

	return variableCollection;
}

QVector<Variable *> * VariableCollection::getVariablesInCategory(const QString & category) const {
	QByteArray categoryBytes = category.toLocal8Bit();

	return getVariablesInCategory(categoryBytes.data());
}
#elif USE_STL
std::vector<Variable *> * VariableCollection::getVariablesInCategory(const char * category) const {
	int categoryIndex = indexOfCategory(category);

	std::vector<Variable *> * variableCollection = new std::vector<Variable * >;
	
	for(unsigned int i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex) {
			variableCollection->push_back(m_variables[i]);
		}
	}

	return variableCollection;
}

std::vector<Variable *> * VariableCollection::getVariablesInCategory(const std::string & category) const {
	return getVariablesInCategory(category.data());
}
#endif // USE_QT

void VariableCollection::setValue(const char * id, const char * value) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return;
	}

	bool valueUpdated = false;

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(id, m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			valueUpdated = true;
		}
	}

	// if the variable doesn't exist, add it
	if(!valueUpdated) {
		addVariable(id, value);
	}

	delete [] formattedID;
}

void VariableCollection::setValue(const char * id, int value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const char * id, double value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const char * id, bool value) {
	setValue(id, value ? "true" : "false");
}

#if USE_QT
void VariableCollection::setValue(const QString & id, const char * value) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	bool valueUpdated = false;

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			valueUpdated = true;
		}
	}

	// if the variable doesn't exist, add it
	if(!valueUpdated) {
		addVariable(id, value);
	}
}

void VariableCollection::setValue(const QString & id, const QString & value) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	bool valueUpdated = false;

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			valueUpdated = true;
		}
	}

	// if the variable doesn't exist, add it
	if(!valueUpdated) {
		addVariable(id, value);
	}
}

void VariableCollection::setValue(const QString & id, int value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const QString & id, double value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const QString & id, bool value) {
	setValue(id, value ? "true" : "false");
}
#endif // USE_QT

#if USE_STL
void VariableCollection::setValue(const std::string & id, const char * value) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return; }

	bool valueUpdated = false;

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			valueUpdated = true;
		}
	}

	// if the variable doesn't exist, add it
	if(!valueUpdated) {
		addVariable(id, value);
	}
}

void VariableCollection::setValue(const std::string & id, const std::string & value) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return; }

	bool valueUpdated = false;

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			valueUpdated = true;
		}
	}

	// if the variable doesn't exist, add it
	if(!valueUpdated) {
		addVariable(id, value);
	}
}

void VariableCollection::setValue(const std::string & id, int value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const std::string & id, double value) {
	setValue(id, Utilities::toString(value));
}

void VariableCollection::setValue(const std::string & id, bool value) {
	setValue(id, value ? "true" : "false");
}
#endif // USE_STL

void VariableCollection::setValue(const char * id, const char * value, const char * category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return;
	}

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(id, m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			delete [] formattedID;
			return;
		}
	}

	// if the variable doesn't exist, add it
	addVariable(id, value, category);

	delete [] formattedID;
}

void VariableCollection::setValue(const char * id, int value, const char * category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const char * id, double value, const char * category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const char * id, bool value, const char * category) {
	setValue(id, value ? "true" : "false", category);
}

#if USE_QT
void VariableCollection::setValue(const QString & id, const char * value, const QString & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			return;
		}
	}

	// if the variable doesn't exist, add it
	addVariable(id, value, category);
}

void VariableCollection::setValue(const QString & id, const QString & value, const QString & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			return;
		}
	}

	// if the variable doesn't exist, add it
	addVariable(id, value, category);
}

void VariableCollection::setValue(const QString & id, int value, const QString & category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const QString & id, double value, const QString & category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const QString & id, bool value, const QString & category) {
	setValue(id, value ? "true" : "false", category);
}
#endif // USE_QT

#if USE_STL
void VariableCollection::setValue(const std::string & id, const char * value, const std::string & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			return;
		}
	}

	// if the variable doesn't exist, add it
	addVariable(id, value, category);
}

void VariableCollection::setValue(const std::string & id, const std::string & value, const std::string & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			m_variables[i]->setValue(value);
			return;
		}
	}

	// if the variable doesn't exist, add it
	addVariable(id, value, category);
}

void VariableCollection::setValue(const std::string & id, int value, const std::string & category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const std::string & id, double value, const std::string & category) {
	setValue(id, Utilities::toString(value), category);
}

void VariableCollection::setValue(const std::string & id, bool value, const std::string & category) {
	setValue(id, value ? "true" : "false", category);
}
#endif // USE_STL

bool VariableCollection::addVariable(const char * id, const char * value, const char * category) {
	if(id == NULL || Utilities::stringLength(id) == 0) { return false; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return false;
	}

	if(!hasVariable(formattedID, category)) {
		int categoryIndex = addCategory(category);
		m_variables.push_back(new Variable(formattedID, value, categoryIndex));

		delete [] formattedID;

		return true;
	}

	delete [] formattedID;

	return false;
}

#if USE_QT
bool VariableCollection::addVariable(const QString & id, const QString & value, const QString & category) {
	if(id.isEmpty()) { return false; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return false; }

	if(!hasVariable(formattedID, category)) {
		int categoryIndex = addCategory(category);
		m_variables.push_back(new Variable(formattedID, value, categoryIndex));

		return true;
	}
	return false;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::addVariable(const std::string & id, const std::string & value, const std::string & category) {
	if(id.empty()) { return false; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return false; }

	if(!hasVariable(formattedID, category)) {
		int categoryIndex = addCategory(category);
		m_variables.push_back(new Variable(formattedID, value, categoryIndex));

		return true;
	}
	return false;
}
#endif // USE_STL

bool VariableCollection::addVariable(Variable * v) {
	if(v == NULL || Utilities::stringLength(v->getID()) == 0) { return false; }

#if USE_STL > USE_QT
	if(!hasVariable(v) && v->getCategory() < static_cast<int>(m_categories.size())) {
#else
	if(!hasVariable(v) && v->getCategory() < m_categories.size()) {
#endif // USE_STL
		m_variables.push_back(v);
		return true;
	}
	return false;
}

bool VariableCollection::removeVariable(int index) {
#if USE_STL > USE_QT
	if(index < 0 || index >= static_cast<int>(m_variables.size())) { return false; }
#else
	if(index < 0 || index >= m_variables.size()) { return false; }
#endif // USE_STL

	delete m_variables[index];
#if USE_STL > USE_QT
	m_variables.erase(m_variables.begin() + index);
#else
	m_variables.remove(index);
#endif // USE_QT

	return true;
}

bool VariableCollection::removeVariable(const char * id) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id) == 0) { return false; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return false;
	}

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			delete [] formattedID;

			return true;
		}
	}

	delete [] formattedID;

	return false;
}

#if USE_QT
bool VariableCollection::removeVariable(const QString & id) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return false; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return false; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			return true;
		}
	}

	return false;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::removeVariable(const std::string & id) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return false; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			return true;
		}
	}

	return false;
}
#endif // USE_STL

bool VariableCollection::removeVariable(const char * id, const char * category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id == NULL || Utilities::stringLength(id)) { return false; }

	char * formattedID = Utilities::trimCopyString(id);

	if(Utilities::stringLength(formattedID) == 0) {
		delete [] formattedID;
		return false;
	}

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID, m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			delete [] formattedID;

			return true;
		}
	}

	delete [] formattedID;

	return false;
}

#if USE_QT
bool VariableCollection::removeVariable(const QString & id, const QString & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.isEmpty()) { return false; }

	QString formattedID = id.trimmed();

	if(formattedID.isEmpty()) { return false; }

	QByteArray formattedIDBytes = formattedID.toLocal8Bit();

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedIDBytes.data(), m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			return true;
		}
	}

	return false;
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::removeVariable(const std::string & id, const std::string & category) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(id.empty()) { return false; }

	std::string formattedID = Utilities::trimString(id);

	if(formattedID.empty()) { return false; }

	int categoryIndex = indexOfCategory(category);

	for(i=0;i<m_variables.size();i++) {
		if(m_variables[i]->getCategory() == categoryIndex &&
		   Utilities::compareStringsIgnoreCase(formattedID.data(), m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			return true;
		}
	}

	return false;
}
#endif // USE_STL

bool VariableCollection::removeVariable(const Variable * v) {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(v == NULL) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(v->getCategory() == m_variables[i]->getCategory() &&
		   Utilities::compareStringsIgnoreCase(v->getID(), m_variables[i]->getID()) == 0) {
			delete m_variables[i];
#if USE_STL > USE_QT
			m_variables.erase(m_variables.begin() + i);
#else
			m_variables.remove(i);
#endif // USE_STL

			return true;
		}
	}

	return false;
}

void VariableCollection::clearVariables() {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	for(i=0;i<m_categories.size();i++) {
		delete [] m_categories[i];
	}
	m_categories.clear();

	for(i=0;i<m_variables.size();i++) {
		delete m_variables[i];
	}
	m_variables.clear();
}

void VariableCollection::sortVariables() {
	m_variables = mergeSort(m_variables);
}

#if USE_QT >= USE_STL
QVector<Variable *> VariableCollection::mergeSort(QVector<Variable *> variables) {
	if(variables.size() <= 1) {
		return variables;
	}

	QVector<Variable *> left;
	QVector<Variable *> right;

	int mid = variables.size() / 2;

	for(int i=0;i<mid;i++) {
		left.push_back(variables[i]);
	}

	for(int i=mid;i<variables.size();i++) {
		right.push_back(variables[i]);
	}

	left = mergeSort(left);
	right = mergeSort(right);

	return merge(left, right);
}

QVector<Variable *> VariableCollection::merge(QVector<Variable *> left, QVector<Variable *> right) {
	QVector<Variable *> result;

	while(left.size() > 0 && right.size() > 0) {
		if(left[0]->getCategory() <= right[0]->getCategory()) {
			result.push_back(left[0]);
			left.remove(0);
		}
		else {
			result.push_back(right[0]);
			right.remove(0);
		}
	}

	for(int i=0;i<left.size();i++) {
		result.push_back(left[i]);
	}

	for(int i=0;i<right.size();i++) {
		result.push_back(right[i]);
	}

	return result;
}
#elif USE_STL
std::vector<Variable *> VariableCollection::mergeSort(std::vector<Variable *> variables) {
	if(variables.size() <= 1) {
		return variables;
	}

	std::vector<Variable *> left;
	std::vector<Variable *> right;

	unsigned int mid = variables.size() / 2;

	for(unsigned int i=0;i<mid;i++) {
		left.push_back(variables[i]);
	}

	for(unsigned int i=mid;i<variables.size();i++) {
		right.push_back(variables[i]);
	}

	left = mergeSort(left);
	right = mergeSort(right);

	return merge(left, right);
}

std::vector<Variable *> VariableCollection::merge(std::vector<Variable *> left, std::vector<Variable *> right) {
	std::vector<Variable *> result;

	while(left.size() > 0 && right.size() > 0) {
		if(left[0]->getCategory() <= right[0]->getCategory()) {
			result.push_back(left[0]);
			left.erase(left.begin());
		}
		else {
			result.push_back(right[0]);
			right.erase(right.begin());
		}
	}

	for(unsigned int i=0;i<left.size();i++) {
		result.push_back(left[i]);
	}

	for(unsigned int i=0;i<right.size();i++) {
		result.push_back(right[i]);
	}

	return result;
}
#endif // USE_QT

#if USE_QT >= USE_STL
VariableCollection * VariableCollection::readFrom(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	QString line;
	char * formattedLine;
	QByteArray bytes;

	// open the file
	QFile input(fileName);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return NULL; }

	VariableCollection * variables = new VariableCollection();
	char * category = NULL;
	int categoryIndex = Variable::NO_CATEGORY;

	// read to the end of the file
	while(true) {
		if(input.atEnd()) { break; }
		
		line = input.readLine().trimmed();
		bytes = line.toLocal8Bit();
		formattedLine = Utilities::trimCopyString(bytes.data());
		
		if(Utilities::stringLength(formattedLine) == 0) {
			if(category != NULL) { delete [] category; }
			category = NULL;
			categoryIndex = Variable::NO_CATEGORY;
			delete [] formattedLine;
			continue;
		}

		// parse a category
		if(Utilities::stringLength(formattedLine) >= 2 && formattedLine[0] == '[' && formattedLine[Utilities::stringLength(formattedLine) - 1] == ']') {
			if(category != NULL) { delete [] category; }

			category = new char[Utilities::stringLength(formattedLine) - 1];
			for(int i=1;i<(int) Utilities::stringLength(formattedLine) - 1;i++) {
				category[i-1] = formattedLine[i];
			}
			category[Utilities::stringLength(formattedLine)-2] = '\0';
			categoryIndex = variables->addCategory(category);
		}
		// parse a variable
		else {
			Variable * v = Variable::parseFrom(formattedLine);
			if(v != NULL) {
				v->setCategory(categoryIndex);
				variables->addVariable(v);
			}
		}

		delete [] formattedLine;
	}

	if(category != NULL) { delete[] category; }

	input.close();

	return variables;
}

VariableCollection * VariableCollection::readFrom(const QString & fileName) {
	if(fileName.isEmpty()) { return NULL; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return readFrom(fileNameBytes.data());
}
#elif USE_STL
VariableCollection * VariableCollection::readFrom(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	char data[1024];
	char * formattedData;

	// open the file
	std::ifstream input(fileName);
	if(!input.is_open()) { return NULL; }

	VariableCollection * variables = new VariableCollection();
	char * category = NULL;
	int categoryIndex = Variable::NO_CATEGORY;

	// read to the end of the file
	while(true) {
		input.getline(data, 1024);
		if(input.eof()) { break; }

		formattedData = Utilities::trimCopyString(data);
		if(Utilities::stringLength(formattedData) == 0) {
			if(category != NULL) { delete [] category; }
			category = NULL;
			categoryIndex = Variable::NO_CATEGORY;
			continue;
		}

		// parse a category
		if(Utilities::stringLength(formattedData) >= 2 && formattedData[0] == '[' && formattedData[Utilities::stringLength(formattedData) - 1] == ']') {
			category = new char[Utilities::stringLength(formattedData) - 1];
			for(unsigned int i=1;i<Utilities::stringLength(formattedData) - 1;i++) {
				category[i-1] = formattedData[i];
			}
			category[Utilities::stringLength(formattedData)-2] = '\0';
			categoryIndex = variables->addCategory(category);
		}
		// parse a variable
		else {
			Variable * v = Variable::parseFrom(formattedData);
			if(v != NULL) {
				v->setCategory(categoryIndex);
				variables->addVariable(v);
			}
		}

		delete [] formattedData;
	}

	if(category != NULL) { delete[] category; }

	input.close();

	return variables;
}
#endif // USE_QT

#if USE_STL
VariableCollection * VariableCollection::readFrom(const std::string & fileName) {
	if(fileName.empty()) { return NULL; }

	return readFrom(fileName.data());
}
#endif // USE_STL

#if USE_QT >= USE_STL
bool VariableCollection::writeTo(const char * fileName) const {
	int lastCategory = Variable::NO_CATEGORY;
	bool firstLine = true;
	QString data;
	
	QFile output(fileName);
	if(!output.open(QIODevice::WriteOnly)) { return NULL; }
	
	for(int i=0;i<m_variables.size();i++) {
		if(lastCategory == Variable::NO_CATEGORY || lastCategory != m_variables[i]->getCategory()) {
			if(m_variables[i]->getCategory() != Variable::NO_CATEGORY) {
				data.clear();
				if(!firstLine) { data.append(Utilities::newLine); }
				data.append('[');
				data.append(m_categories[m_variables[i]->getCategory()]);
				data.append(']');
				data.append(Utilities::newLine);
				output.write(data.toLocal8Bit().data(), data.length());
				firstLine = false;
			}
			lastCategory = m_variables[i]->getCategory();
		}
		data.clear();
		data.append(m_variables[i]->getID());
		data.append(Variable::SEPARATORS[0]);
		data.append(" ");
		data.append(m_variables[i]->getValue());
		data.append(Utilities::newLine);
		output.write(data.toLocal8Bit().data(), data.length());
		firstLine = false;
	}
	
	output.close();
	
	return true;
}
#elif USE_STL
bool VariableCollection::writeTo(const char * fileName) const {
	int lastCategory = Variable::NO_CATEGORY;
	bool firstLine = true;

	std::ofstream output(fileName);
	if(!output.is_open()) {
		return false;
	}

	for(unsigned int i=0;i<m_variables.size();i++) {
		if(lastCategory == Variable::NO_CATEGORY || lastCategory != m_variables[i]->getCategory()) {
			if(m_variables[i]->getCategory() != Variable::NO_CATEGORY) {
				if(!firstLine) { output << Utilities::newLine; }
				output << "[" << m_categories[m_variables[i]->getCategory()] << "]" << Utilities::newLine;
				firstLine = false;
			}
			lastCategory = m_variables[i]->getCategory();
		}
		output << m_variables[i]->getID() << Variable::SEPARATORS[0] << " " << m_variables[i]->getValue() << Utilities::newLine;
		firstLine = false;
	}

	output.close();

	return true;
}
#endif // USE_QT

#if USE_QT
bool VariableCollection::writeTo(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return writeTo(fileNameBytes.data());
}
#endif // USE_QT

#if USE_STL
bool VariableCollection::writeTo(const std::string & fileName) const {
	if(fileName.empty()) { return NULL; }

	return writeTo(fileName.data());
}
#endif // USE_STL

bool VariableCollection::operator == (const VariableCollection & v) const {
#if USE_STL > USE_QT
	unsigned int i;
#else
	int i;
#endif // USE_STL

	if(m_variables.size() != v.m_variables.size()) { return false; }

	for(i=0;i<m_variables.size();i++) {
		if(!v.hasVariable(m_variables[i])) {
			return false;
		}
	}
	return true;
}

bool VariableCollection::operator != (const VariableCollection & v) const {
	return !operator == (v);
}
