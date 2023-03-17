#include "SettingPanel.h"

#include "WXUtilities.h"

#include <spdlog/spdlog.h>
#include <wx/wrapsizer.h>

#include <sstream>

SettingPanel::SettingPanel(const std::string & name, wxWindow * parent)
	: wxPanel(parent)
	, m_name(name)
	, m_modified(false)
	, m_editable(true) { }

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

bool SettingPanel::isEditable() const {
	return m_editable;
}

void SettingPanel::setEditable(bool editable) {
	if(m_editable == editable) {
		return;
	}

	m_editable = editable;

	m_setEditableFunction(editable);
}

void SettingPanel::update() {
	if(m_modified) {
		return;
	}

	discard();
}

bool SettingPanel::save() {
	if(!m_modified) {
		return true;
	}

	if(!isValid()) {
		return false;
	}

	m_saveFunction();

	setModified(false);

	return true;
}

void SettingPanel::discard() {
	m_discardFunction();

	setModified(false);
}

void SettingPanel::reset() {
	m_resetFunction();

	setModified(true);
}

void SettingPanel::setModified(bool modified) {
	m_modified = modified;

	settingModified(*this);
}

SettingPanel * SettingPanel::createBooleanSettingPanel(bool & setting, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	return createBooleanSettingPanel([&setting]() { return setting; }, [&setting](bool newSetting) { setting = newSetting; }, defaultSetting, name, parent, parentSizer);
}

SettingPanel * SettingPanel::createBooleanSettingPanel(std::function<bool()> getSettingValueFunction, std::function<void(bool)> setSettingValueFunction, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * settingCheckBox = new wxCheckBox(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, name);
	settingCheckBox->SetFont(settingCheckBox->GetFont().MakeBold());
	settingCheckBox->SetValue(getSettingValueFunction());
	settingCheckBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingCheckBox]() {
		return settingCheckBox->GetValue() ? "true" : "false";
	};

	settingPanel->m_saveFunction = [settingCheckBox, setSettingValueFunction]() {
		setSettingValueFunction(settingCheckBox->GetValue());
	};

	settingPanel->m_discardFunction = [settingCheckBox, getSettingValueFunction]() {
		settingCheckBox->SetValue(getSettingValueFunction());
	};

	settingPanel->m_resetFunction = [settingCheckBox, defaultSetting]() {
		settingCheckBox->SetValue(defaultSetting);
	};

	settingPanel->m_setEditableFunction = [settingCheckBox](bool editable) {
		if(editable) {
			settingCheckBox->Enable();
		}
		else {
			settingCheckBox->Disable();
		}
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingCheckBox, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

SettingPanel * SettingPanel::createOptionalBooleanSettingPanel(std::optional<bool> & setting, std::optional<bool> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	return createOptionalBooleanSettingPanel([&setting]() { return setting; }, [&setting](bool newSetting) { setting = newSetting; }, [&setting]() { setting.reset(); }, defaultSetting, name, parent, parentSizer);
}

SettingPanel * SettingPanel::createOptionalBooleanSettingPanel(std::function<std::optional<bool>()> getSettingValueFunction, std::function<void(bool)> setSettingValueFunction, std::function<void()> clearSettingValueFunction, std::optional<bool> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	std::optional<bool> setting(getSettingValueFunction());

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * settingCheckBox = new wxCheckBox(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, name);
	settingCheckBox->SetFont(settingCheckBox->GetFont().MakeBold());
	settingCheckBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	if(setting.has_value()) {
		settingCheckBox->SetValue(setting.value());
	}

	settingPanel->m_getValueFunction = [settingCheckBox]() {
		return settingCheckBox->GetValue() ? "true" : "false";
	};

	settingPanel->m_saveFunction = [settingCheckBox, setSettingValueFunction]() {
		setSettingValueFunction(settingCheckBox->GetValue());
	};

	settingPanel->m_discardFunction = [settingPanel, settingCheckBox, getSettingValueFunction]() {
		settingCheckBox->SetValue(getSettingValueFunction().value_or(false));
	};

	settingPanel->m_resetFunction = [settingPanel, settingCheckBox, defaultSetting]() {
		settingCheckBox->SetValue(defaultSetting.value_or(false));
	};

	settingPanel->m_setEditableFunction = [settingCheckBox](bool editable) {
		if(editable) {
			settingCheckBox->Enable();
		}
		else {
			settingCheckBox->Disable();
		}
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingCheckBox, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

SettingPanel * SettingPanel::createStringSettingPanel(std::string & setting, std::string defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength, size_t maxLength, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	return createStringSettingPanel<void>([&setting]() -> const std::string & { return setting; }, [&setting](const std::string & newSetting) { setting = newSetting; }, defaultSetting, name, parent, parentSizer, minLength, maxLength, customValidatorFunction);
}

SettingPanel * SettingPanel::createOptionalStringSettingPanel(std::optional<std::string> & setting, std::optional<std::string> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength, size_t maxLength, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	return createOptionalStringSettingPanel<void>([&setting]() { return setting; }, [&setting](const std::string & newSetting) { setting = newSetting; }, [&setting]() { setting.reset(); }, defaultSetting, name, parent, parentSizer, minLength, maxLength, customValidatorFunction);
}

StringChoiceSettingPanel * SettingPanel::createStringChoiceSettingPanel(std::string & setting, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer) {
	return createStringChoiceSettingPanel([&setting]() { return setting; }, [&setting](const std::string & newSetting) { setting = newSetting; return true; }, defaultSetting, name, choices, parent, parentSizer);
}

StringChoiceSettingPanel * SettingPanel::createStringChoiceSettingPanel(std::function<std::string()> getSettingValueFunction, std::function<bool(const std::string &)> setSettingValueFunction, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	StringChoiceSettingPanel * settingPanel = new StringChoiceSettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxComboBox * settingComboBox = new wxComboBox(settingPanel, wxID_ANY, getSettingValueFunction(), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(choices), 0, wxDefaultValidator, name);
	settingComboBox->SetEditable(false);
	settingComboBox->Bind(wxEVT_COMBOBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingComboBox]() {
		return settingComboBox->GetValue();
	};

	settingPanel->m_saveFunction = [settingComboBox, setSettingValueFunction]() {
		setSettingValueFunction(settingComboBox->GetValue());
	};

	settingPanel->m_discardFunction = [settingComboBox, getSettingValueFunction]() {
		settingComboBox->ChangeValue(getSettingValueFunction());
	};

	settingPanel->m_resetFunction = [settingComboBox, defaultSetting]() {
		settingComboBox->ChangeValue(defaultSetting);
	};

	settingPanel->m_setEditableFunction = [settingComboBox](bool editable) {
		settingComboBox->SetEditable(editable);
	};

	settingPanel->m_setChoicesFunction = [settingComboBox](const std::vector<std::string> & choices) {
		settingComboBox->Set(WXUtilities::createItemWXArrayString(choices));
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

SettingPanel * SettingPanel::createStringMultiChoiceSettingPanel(std::vector<std::string> & setting, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer) {
	return createStringMultiChoiceSettingPanel(
		[&setting]() -> const std::vector<std::string> & {
			return setting;
		},
		[&setting, caseSensitive](const std::string & entry) {
			return std::find_if(setting.cbegin(), setting.cend(), [&entry, caseSensitive](const std::string & currentEntry) {
				return Utilities::areStringsEqual(entry, currentEntry, caseSensitive);
			}) != setting.cend();
		},
		[&setting, caseSensitive](const std::string & entry) {
			if(std::find_if(setting.cbegin(), setting.cend(), [&entry, caseSensitive](const std::string & currentEntry) {
				return Utilities::areStringsEqual(entry, currentEntry, caseSensitive);
			}) != setting.cend()) {
				return false;
			}

			setting.push_back(entry);

			return true;
		},
		[&setting, caseSensitive](const std::string & entry) {
			auto entryIterator = std::find_if(setting.cbegin(), setting.cend(), [&entry, caseSensitive](const std::string & currentEntry) {
				return Utilities::areStringsEqual(entry, currentEntry, caseSensitive);
			});

			if(entryIterator == setting.cend()) {
				return false;
			}

			setting.erase(entryIterator);

			return true;
		},
		name,
		caseSensitive,
		choices,
		parent,
		minimumNumberOfSelectedItems,
		parentSizer
	);
}

SettingPanel * SettingPanel::createStringMultiChoiceSettingPanel(std::function<const std::vector<std::string> &()> getSettingValueFunction, std::function<bool(const std::string &)> hasSettingEntryFunction, std::function<bool(const std::string &)> addSettingEntryFunction, std::function<bool(const std::string &)> removeSettingEntryFunction, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * checkBox = nullptr;
	std::vector<wxCheckBox *> checkBoxes;
	checkBoxes.reserve(choices.size());

	int border = 5;

	wxWrapSizer * settingSizer = new wxWrapSizer(wxHORIZONTAL);

	for(const std::string & choice : choices) {
		checkBox = new wxCheckBox(settingPanel, wxID_ANY, choice, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, choice);
		checkBox->SetFont(checkBox->GetFont().MakeBold());
		checkBox->SetValue(hasSettingEntryFunction(choice));
		checkBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

		checkBoxes.push_back(checkBox);

		settingSizer->Add(checkBox, 1, wxEXPAND | wxHORIZONTAL, border);
	}

	settingPanel->m_getValueFunction = [checkBoxes, choices]() {
		std::stringstream valueStream;

		for(size_t i = 0; i < choices.size(); i++) {
			if(!checkBoxes[i]->GetValue()) {
				continue;
			}

			if(valueStream.tellp() != 0) {
				valueStream << ", ";
			}

			valueStream << choices[i];
		}

		return valueStream.str();
	};

	settingPanel->m_defaultValidatorFunction = [checkBoxes, minimumNumberOfSelectedItems]() {
		if(minimumNumberOfSelectedItems == 0) {
			return true;
		}

		size_t numberOfSelectedItems = 0;

		for(const wxCheckBox * checkBox : checkBoxes) {
			if(checkBox->GetValue()) {
				numberOfSelectedItems++;
			}

			if(numberOfSelectedItems >= minimumNumberOfSelectedItems) {
				return true;
			}
		}

		return false;
	};

	settingPanel->m_saveFunction = [checkBoxes, choices, hasSettingEntryFunction, addSettingEntryFunction, removeSettingEntryFunction]() {
		for(size_t i = 0; i < choices.size(); i++) {
			bool hasEntry = hasSettingEntryFunction(choices[i]);

			if(checkBoxes[i]->GetValue()) {
				if(!hasEntry) {
					addSettingEntryFunction(choices[i]);
				}
			}
			else {
				if(hasEntry) {
					removeSettingEntryFunction(choices[i]);
				}
			}
		}
	};

	settingPanel->m_discardFunction = [checkBoxes, choices, hasSettingEntryFunction]() {
		for(size_t i = 0; i < choices.size(); i++) {
			checkBoxes[i]->SetValue(hasSettingEntryFunction(choices[i]));
		}
	};

	settingPanel->m_resetFunction = [checkBoxes, choices]() {
		for(size_t i = 0; i < choices.size(); i++) {
			checkBoxes[i]->SetValue(false);
		}
	};

	settingPanel->m_setEditableFunction = [checkBoxes, choices](bool editable) {
		for(size_t i = 0; i < choices.size(); i++) {
			if(editable) {
				checkBoxes[i]->Enable();
			}
			else {
				checkBoxes[i]->Disable();
			}
		}
	};

	settingPanel->SetSizer(settingSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

StringChoiceSettingPanel::StringChoiceSettingPanel(const std::string & name, wxWindow * parent)
	: SettingPanel(name, parent) { }

StringChoiceSettingPanel::~StringChoiceSettingPanel() { }

bool StringChoiceSettingPanel::setChoices(const std::vector<std::string> & choices) {
	if(m_setChoicesFunction == nullptr) {
		return false;
	}

	m_setChoicesFunction(choices);

	discard();

	return true;
}
