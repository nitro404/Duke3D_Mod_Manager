#include "SettingPanel.h"

#include "WXUtilities.h"

SettingPanel::SettingPanel(const std::string & name, wxWindow * parent)
	: wxPanel(parent)
	, m_name(name)
	, m_modified(false) { }

SettingPanel::~SettingPanel() { }

const std::string & SettingPanel::getName() const {
	return m_name;
}

std::string SettingPanel::getValue() const {
	return m_getValueFunction();
}

bool SettingPanel::isModified() const {
	return m_modified;
}

bool SettingPanel::isValid() const {
	if(m_defaultValidatorFunction != nullptr && !m_defaultValidatorFunction()) {
		return false;
	}

	if(m_customValidatorFunction != nullptr && !m_customValidatorFunction(this)) {
		return false;
	}

	return true;
}

bool SettingPanel::save() {
	return m_saveFunction();
}

void SettingPanel::discard() {
	m_discardFunction();
}

SettingPanel::Listener::~Listener() { }

size_t SettingPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool SettingPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t SettingPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

SettingPanel::Listener * SettingPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool SettingPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool SettingPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool SettingPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void SettingPanel::clearListeners() {
	m_listeners.clear();
}

void SettingPanel::setModified(bool modified) {
	m_modified = modified;

	notifySettingModified();
}

void SettingPanel::notifySettingModified() {
	for(Listener * listener : m_listeners) {
		listener->settingModified(this);
	}
}

SettingPanel * SettingPanel::createBooleanSettingPanel(bool & setting, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * settingCheckBox = new wxCheckBox(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, name);
	settingCheckBox->SetFont(settingCheckBox->GetFont().MakeBold());
	settingCheckBox->SetValue(setting);
	settingCheckBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingCheckBox]() {
		return settingCheckBox->GetValue() ? "true" : "false";
	};

	settingPanel->m_saveFunction = [settingPanel, settingCheckBox, &setting]() {
		setting = settingCheckBox->GetValue();
		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingCheckBox, &setting]() {
		settingCheckBox->SetValue(setting);
		settingPanel->setModified(false);
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingCheckBox, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

SettingPanel * SettingPanel::createStringSettingPanel(std::string & setting, const std::string & defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength, size_t maxLength, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_customValidatorFunction = customValidatorFunction;

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, setting, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
	settingTextField->SetMinSize(wxSize(150, settingTextField->GetMinSize().y));
	settingTextField->Bind(wxEVT_TEXT, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_defaultValidatorFunction = [settingTextField, minLength, maxLength]() {
		size_t valueLength = settingTextField->GetValue().Length();

		return valueLength >= minLength && valueLength <= maxLength;
	};

	settingPanel->m_getValueFunction = [settingTextField]() {
		return settingTextField->GetValue();
	};

	settingPanel->m_saveFunction = [settingPanel, settingTextField, &setting]() {
		if(!settingPanel->isValid()) {
			return false;
		}

		setting = settingTextField->GetValue();
		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingTextField, &setting]() {
		settingTextField->SetValue(setting);
		settingPanel->setModified(false);
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingLabel, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingBoxSizer->Add(settingTextField, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

SettingPanel * SettingPanel::createStringChoiceSettingPanel(std::string & setting, const std::string & defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxComboBox * settingComboBox = new wxComboBox(settingPanel, wxID_ANY, setting, wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(choices), 0, wxDefaultValidator, name);
	settingComboBox->SetEditable(false);
	settingComboBox->Bind(wxEVT_COMBOBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingComboBox]() {
		return settingComboBox->GetValue();
	};

	settingPanel->m_saveFunction = [settingPanel, settingComboBox, &setting]() {
		setting = settingComboBox->GetValue();
		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingComboBox, &setting]() {
		settingComboBox->SetValue(setting);
		settingPanel->setModified(false);
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingLabel, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingBoxSizer->Add(settingComboBox, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}
