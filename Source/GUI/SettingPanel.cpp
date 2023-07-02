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

StringChoiceSettingPanel * SettingPanel::createStringChoiceSettingPanel(std::string & setting, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer, const std::vector<std::string> & values) {
	return createStringChoiceSettingPanel([&setting]() { return setting; }, [&setting](const std::string & newSetting) { setting = newSetting; return true; }, defaultSetting, name, choices, parent, parentSizer, values);
}

StringChoiceSettingPanel * SettingPanel::createStringChoiceSettingPanel(std::function<std::string()> getSettingValueFunction, std::function<bool(const std::string &)> setSettingValueFunction, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer, const std::vector<std::string> & values) {
	if(parent == nullptr || (!values.empty() && choices.size() != values.size())) {
		return nullptr;
	}

	StringChoiceSettingPanel * settingPanel = new StringChoiceSettingPanel(name, parent);
	settingPanel->m_values = values.empty() ? choices : values;

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxComboBox * settingComboBox = new wxComboBox(settingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(choices), 0, wxDefaultValidator, name);
	std::vector<std::string>::const_iterator currentSettingValueIterator(std::find(settingPanel->m_values.cbegin(), settingPanel->m_values.cend(), getSettingValueFunction()));
	settingComboBox->SetSelection(currentSettingValueIterator != settingPanel->m_values.cend() ? currentSettingValueIterator - settingPanel->m_values.cbegin() : wxNOT_FOUND);
	settingComboBox->SetEditable(false);
	settingComboBox->Bind(wxEVT_COMBOBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingPanel, settingComboBox]() {
		int currentSelectionIndex = settingComboBox->GetSelection();

		if(currentSelectionIndex == wxNOT_FOUND) {
			return Utilities::emptyString;
		}

		return settingPanel->m_values[currentSelectionIndex];
	};

	settingPanel->m_saveFunction = [settingPanel, settingComboBox, setSettingValueFunction]() {
		int currentSelectionIndex = settingComboBox->GetSelection();

		if(currentSelectionIndex == wxNOT_FOUND) {
			return;
		}

		setSettingValueFunction(settingPanel->m_values[currentSelectionIndex]);
	};

	settingPanel->m_discardFunction = [settingPanel, settingComboBox, getSettingValueFunction]() {
		std::vector<std::string>::const_iterator currentSettingValueIterator(std::find(settingPanel->m_values.cbegin(), settingPanel->m_values.cend(), getSettingValueFunction()));

		if(currentSettingValueIterator == settingPanel->m_values.cend()) {
			return;
		}

		settingComboBox->SetSelection(currentSettingValueIterator - settingPanel->m_values.cbegin());
	};

	settingPanel->m_resetFunction = [settingPanel, settingComboBox, defaultSetting]() {
		std::vector<std::string>::const_iterator defaultSettingValueIterator(std::find(settingPanel->m_values.cbegin(), settingPanel->m_values.cend(), defaultSetting));

		if(defaultSettingValueIterator == settingPanel->m_values.cend()) {
			return;
		}

		settingComboBox->SetSelection(defaultSettingValueIterator - settingPanel->m_values.cbegin());
	};

	settingPanel->m_setEditableFunction = [settingComboBox](bool editable) {
		settingComboBox->SetEditable(editable);
	};

	settingPanel->m_setChoicesFunction = [settingPanel, settingComboBox](const std::vector<std::string> & choices, const std::vector<std::string> & values) {
		settingPanel->m_values = values.empty() ? choices : values;

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

SettingPanel * SettingPanel::createStringMultiChoiceSettingPanel(std::vector<std::string> & setting, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer, const std::vector<std::string> & values, const std::vector<std::string> & defaultValues) {
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
		parentSizer,
		values,
		defaultValues
	);
}

SettingPanel * SettingPanel::createStringMultiChoiceSettingPanel(std::function<const std::vector<std::string> &()> getSettingValueFunction, std::function<bool(const std::string &)> hasSettingEntryFunction, std::function<bool(const std::string &)> addSettingEntryFunction, std::function<bool(const std::string &)> removeSettingEntryFunction, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer, const std::vector<std::string> & values, const std::vector<std::string> & defaultValues) {
	if(parent == nullptr || (!values.empty() && choices.size() != values.size())) {
		return nullptr;
	}

	const std::vector<std::string> & actualValues = values.empty() ? choices : values;

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * checkBox = nullptr;
	std::vector<wxCheckBox *> checkBoxes;
	checkBoxes.reserve(choices.size());

	int border = 5;

	wxWrapSizer * settingSizer = new wxWrapSizer(wxHORIZONTAL);

	for(size_t i = 0; i < choices.size(); i++) {
		checkBox = new wxCheckBox(settingPanel, wxID_ANY, wxString::FromUTF8(choices[i]), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, choices[i]);
		checkBox->SetFont(checkBox->GetFont().MakeBold());
		checkBox->SetValue(hasSettingEntryFunction(actualValues[i]));
		checkBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

		checkBoxes.push_back(checkBox);

		settingSizer->Add(checkBox, 1, wxEXPAND | wxHORIZONTAL, border);
	}

	settingPanel->m_getValueFunction = [checkBoxes, actualValues]() {
		std::stringstream valueStream;

		for(size_t i = 0; i < checkBoxes.size(); i++) {
			if(!checkBoxes[i]->GetValue()) {
				continue;
			}

			if(valueStream.tellp() != 0) {
				valueStream << ", ";
			}

			valueStream << actualValues[i];
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

	settingPanel->m_saveFunction = [checkBoxes, actualValues, hasSettingEntryFunction, addSettingEntryFunction, removeSettingEntryFunction]() {
		for(size_t i = 0; i < checkBoxes.size(); i++) {
			bool hasEntry = hasSettingEntryFunction(actualValues[i]);

			if(checkBoxes[i]->GetValue()) {
				if(!hasEntry) {
					addSettingEntryFunction(actualValues[i]);
				}
			}
			else {
				if(hasEntry) {
					removeSettingEntryFunction(actualValues[i]);
				}
			}
		}
	};

	settingPanel->m_discardFunction = [checkBoxes, actualValues, hasSettingEntryFunction]() {
		for(size_t i = 0; i < checkBoxes.size(); i++) {
			checkBoxes[i]->SetValue(hasSettingEntryFunction(actualValues[i]));
		}
	};

	settingPanel->m_resetFunction = [checkBoxes, actualValues, defaultValues]() {
		for(size_t i = 0; i < checkBoxes.size(); i++) {
			checkBoxes[i]->SetValue(std::find(defaultValues.cbegin(), defaultValues.cend(), actualValues[i]) != defaultValues.cend());
		}
	};

	settingPanel->m_setEditableFunction = [checkBoxes](bool editable) {
		for(size_t i = 0; i < checkBoxes.size(); i++) {
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

bool StringChoiceSettingPanel::setChoices(const std::vector<std::string> & choices, const std::vector<std::string> & values) {
	if(m_setChoicesFunction == nullptr) {
		return false;
	}

	m_setChoicesFunction(choices, values);

	discard();

	return true;
}
