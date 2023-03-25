#ifndef _SETTING_PANEL_H_
#define _SETTING_PANEL_H_

#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <boost/signals2.hpp>
#include <fmt/core.h>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class StringChoiceSettingPanel;

class SettingPanel : public wxPanel {
public:
	virtual ~SettingPanel();

	const std::string & getName() const;
	std::string getValue() const;
	bool isModified() const;
	bool isValid() const;
	bool isEditable() const;
	void setEditable(bool editable);
	void update();
	bool save();
	void discard();
	void reset();

	static SettingPanel * createBooleanSettingPanel(bool & setting, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	static SettingPanel * createBooleanSettingPanel(std::function<bool()> getSettingValueFunction, std::function<void(bool)> setSettingValueFunction, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	static SettingPanel * createOptionalBooleanSettingPanel(std::optional<bool> & setting, std::optional<bool> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	static SettingPanel * createOptionalBooleanSettingPanel(std::function<std::optional<bool>()> getSettingValueFunction, std::function<void(bool)> setSettingValueFunction, std::function<void()> clearSettingValueFunction, std::optional<bool> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	template <typename T>
	static SettingPanel * createIntegerSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue = std::numeric_limits<T>::min(), size_t maxValue = std::numeric_limits<T>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename T>
	static SettingPanel * createIntegerSettingPanel(std::function<T()> getSettingValueFunction, std::function<void(T)> setSettingValueFunction, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue = std::numeric_limits<T>::min(), size_t maxValue = std::numeric_limits<T>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename T>
	static SettingPanel * createChronoSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	static SettingPanel * createStringSettingPanel(std::string & setting, std::string defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength = 0, size_t maxLength = std::numeric_limits<size_t>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename R>
	static SettingPanel * createStringSettingPanel(std::function<const std::string &()> getSettingValueFunction, std::function<R(const std::string &)> setSettingValueFunction, std::string defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength = 0, size_t maxLength = std::numeric_limits<size_t>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	static SettingPanel * createOptionalStringSettingPanel(std::optional<std::string> & setting, std::optional<std::string> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength = 0, size_t maxLength = std::numeric_limits<size_t>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename R>
	static SettingPanel * createOptionalStringSettingPanel(std::function<std::optional<std::string>()> getSettingValueFunction, std::function<R(const std::string &)> setSettingValueFunction, std::function<void()> clearSettingValueFunction, std::optional<std::string> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength = 0, size_t maxLength = std::numeric_limits<size_t>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename E>
	static SettingPanel * createEnumSettingPanel(E & setting, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, const std::vector<E> & disabledEnumValues);
	template <typename E>
	static SettingPanel * createEnumSettingPanel(std::function<E()> getSettingValueFunction, std::function<void(E)> setSettingValueFunction, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, const std::vector<E> & disabledEnumValues);
	static StringChoiceSettingPanel * createStringChoiceSettingPanel(std::string & setting, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer, const std::vector<std::string> & values = {});
	static StringChoiceSettingPanel * createStringChoiceSettingPanel(std::function<std::string()> getSettingValueFunction, std::function<bool(const std::string &)> setSettingValueFunction, std::string defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer, const std::vector<std::string> & values = {});
	static SettingPanel * createStringMultiChoiceSettingPanel(std::vector<std::string> & setting, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems = 0, wxSizer * parentSizer = nullptr, const std::vector<std::string> & values = {});
	static SettingPanel * createStringMultiChoiceSettingPanel(std::function<const std::vector<std::string> &()> getSettingValueFunction, std::function<bool(const std::string &)> hasSettingEntryFunction, std::function<bool(const std::string &)> addSettingEntryFunction, std::function<bool(const std::string &)> removeSettingEntryFunction, const std::string & name, bool caseSensitive, const std::vector<std::string> & choices, wxWindow * parent, size_t minimumNumberOfSelectedItems = 0, wxSizer * parentSizer = nullptr, const std::vector<std::string> & values = {});
	template <typename E>
	static SettingPanel * createEnumMultiChoiceSettingPanel(std::vector<E> & setting, const std::string & name, wxWindow * parent, size_t minimumNumberOfSelectedItems = 0, wxSizer * parentSizer = nullptr);
	template <typename E>
	static SettingPanel * createEnumMultiChoiceSettingPanel(std::function<const std::vector<E> &()> getSettingValueFunction, std::function<bool(E)> hasSettingEntryFunction, std::function<bool(E)> addSettingEntryFunction, std::function<bool(E)> removeSettingEntryFunction, const std::string & name, wxWindow * parent, size_t minimumNumberOfSelectedItems = 0, wxSizer * parentSizer = nullptr);

	boost::signals2::signal<void (SettingPanel & /* settingPanel */)> settingModified;

protected:
	SettingPanel(const std::string & name, wxWindow * parent);

private:
	void setModified(bool modified);

	std::string m_name;
	bool m_modified;
	bool m_editable;
	std::function<void(wxCommandEvent &)> m_changedFunction;
	std::function<std::string()> m_getValueFunction;
	std::function<bool()> m_defaultValidatorFunction;
	std::function<bool(const SettingPanel *)> m_customValidatorFunction;
	std::function<void()> m_saveFunction;
	std::function<void()> m_discardFunction;
	std::function<void()> m_resetFunction;
	std::function<void(bool)> m_setEditableFunction;
};

class StringChoiceSettingPanel final : public SettingPanel {
	friend class SettingPanel;

public:
	virtual ~StringChoiceSettingPanel();

	bool setChoices(const std::vector<std::string> & choices, const std::vector<std::string> & values = {});

protected:
	StringChoiceSettingPanel(const std::string & name, wxWindow * parent);

private:
	std::vector<std::string> m_values;
	std::function<void(const std::vector<std::string> & choices, const std::vector<std::string> & values)> m_setChoicesFunction;
};

template <typename T>
SettingPanel * SettingPanel::createIntegerSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue, size_t maxValue, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	return createIntegerSettingPanel<T>([&setting]() { return setting; }, [&setting](T newSetting) { setting = newSetting; }, defaultSetting, name, parent, parentSizer, minValue, maxValue, customValidatorFunction);
}

template <typename T>
SettingPanel * SettingPanel::createIntegerSettingPanel(std::function<T()> getSettingValueFunction, std::function<void(T)> setSettingValueFunction, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue, size_t maxValue, std::function<bool(const SettingPanel *)> customValidatorFunction) {
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
	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, std::to_string(getSettingValueFunction()), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
	settingTextField->Bind(wxEVT_TEXT, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingTextField]() {
		return settingTextField->GetValue();
	};

	settingPanel->m_defaultValidatorFunction = [settingPanel, settingTextField, minValue, maxValue]() {
		bool error = false;

		if(std::is_signed<T>()) {
			int64_t value = Utilities::parseLong(settingTextField->GetValue(), &error);

			return !error &&
				   value >= std::numeric_limits<T>::min() &&
				   value <= std::numeric_limits<T>::max() &&
				   value >= minValue &&
				   value <= maxValue;
		}
		else {
			uint64_t value = Utilities::parseUnsignedLong(settingTextField->GetValue(), &error);

			return !error &&
				   value <= std::numeric_limits<T>::max() &&
				   value >= minValue &&
				   value <= maxValue;
		}
	};

	settingPanel->m_saveFunction = [settingTextField, setSettingValueFunction]() {
		if(std::is_signed<T>()) {
			setSettingValueFunction(static_cast<T>(Utilities::parseLong(settingTextField->GetValue(), nullptr)));
		}
		else {
			setSettingValueFunction(static_cast<T>(Utilities::parseUnsignedLong(settingTextField->GetValue(), nullptr)));
		}
	};

	settingPanel->m_discardFunction = [settingTextField, getSettingValueFunction]() {
		settingTextField->ChangeValue(std::to_string(getSettingValueFunction()));
	};

	settingPanel->m_resetFunction = [settingTextField, defaultSetting]() {
		settingTextField->ChangeValue(std::to_string(defaultSetting));
	};

	settingPanel->m_setEditableFunction = [settingTextField](bool editable) {
		settingTextField->SetEditable(editable);
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

template <typename T>
SettingPanel * SettingPanel::createChronoSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_customValidatorFunction = customValidatorFunction;

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	std::string durationName(Utilities::getDurationName<T>());
	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, fmt::format("{} ({})", name, durationName), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, std::to_string(setting.count()), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
	settingTextField->Bind(wxEVT_TEXT, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingTextField]() {
		return settingTextField->GetValue();
	};

	settingPanel->m_defaultValidatorFunction = [settingTextField]() {
		bool error = false;
		int64_t value = Utilities::parseLong(settingTextField->GetValue(), &error);

		return !error &&
			   value >= 0;
	};

	settingPanel->m_saveFunction = [settingTextField, &setting]() {
		setting = T(Utilities::parseLong(settingTextField->GetValue(), nullptr));
	};

	settingPanel->m_discardFunction = [settingTextField, &setting]() {
		settingTextField->ChangeValue(std::to_string(setting.count()));
	};

	settingPanel->m_resetFunction = [settingTextField, defaultSetting]() {
		settingTextField->ChangeValue(std::to_string(defaultSetting.count()));
	};

	settingPanel->m_setEditableFunction = [settingTextField](bool editable) {
		settingTextField->SetEditable(editable);
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

template <typename R>
SettingPanel * SettingPanel::createStringSettingPanel(std::function<const std::string &()> getSettingValueFunction, std::function<R(const std::string &)> setSettingValueFunction, std::string defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength, size_t maxLength, std::function<bool(const SettingPanel *)> customValidatorFunction) {
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
	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, wxString::FromUTF8(getSettingValueFunction()), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
	settingTextField->SetMinSize(wxSize(150, settingTextField->GetMinSize().y));
	settingTextField->Bind(wxEVT_TEXT, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_defaultValidatorFunction = [settingTextField, minLength, maxLength]() {
		size_t valueLength = settingTextField->GetValue().Length();

		return valueLength >= minLength && valueLength <= maxLength;
	};

	settingPanel->m_getValueFunction = [settingTextField]() {
		return settingTextField->GetValue();
	};

	settingPanel->m_saveFunction = [settingTextField, setSettingValueFunction]() {
		setSettingValueFunction(settingTextField->GetValue());
	};

	settingPanel->m_discardFunction = [settingTextField, getSettingValueFunction]() {
		settingTextField->ChangeValue(wxString::FromUTF8(getSettingValueFunction()));
	};

	settingPanel->m_resetFunction = [settingTextField, defaultSetting]() {
		settingTextField->ChangeValue(wxString::FromUTF8(defaultSetting));
	};

	settingPanel->m_setEditableFunction = [settingTextField](bool editable) {
		settingTextField->SetEditable(editable);
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

template <typename R>
SettingPanel * SettingPanel::createOptionalStringSettingPanel(std::function<std::optional<std::string>()> getSettingValueFunction, std::function<R(const std::string &)> setSettingValueFunction, std::function<void()> clearSettingValueFunction, std::optional<std::string> defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength, size_t maxLength, std::function<bool(const SettingPanel *)> customValidatorFunction) {
	if(parent == nullptr) {
		return nullptr;
	}

	std::optional<std::string> setting(getSettingValueFunction());

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_customValidatorFunction = customValidatorFunction;

	wxCheckBox * settingEnabledCheckBox = new wxCheckBox(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, name);
	settingEnabledCheckBox->SetFont(settingEnabledCheckBox->GetFont().MakeBold());
	settingEnabledCheckBox->SetValue(setting.has_value());

	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, wxString::FromUTF8(setting.value_or("")), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
	settingTextField->SetMinSize(wxSize(150, settingTextField->GetMinSize().y));
	WXUtilities::setTextControlEnabled(settingTextField, setting.has_value());

	settingPanel->m_changedFunction = [settingPanel, settingTextField, settingEnabledCheckBox](wxCommandEvent & event) {
		settingPanel->setModified(true);
		WXUtilities::setTextControlEnabled(settingTextField, settingEnabledCheckBox->GetValue());
	};

	settingEnabledCheckBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingTextField->Bind(wxEVT_TEXT, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_defaultValidatorFunction = [settingTextField, settingEnabledCheckBox, minLength, maxLength]() {
		if(!settingEnabledCheckBox->GetValue()) {
			return true;
		}

		size_t valueLength = settingTextField->GetValue().Length();

		return valueLength >= minLength && valueLength <= maxLength;
	};

	settingPanel->m_getValueFunction = [settingTextField, settingEnabledCheckBox]() {
		if(!settingEnabledCheckBox->GetValue()) {
			return Utilities::emptyString;
		}

		return std::string(settingTextField->GetValue());
	};

	settingPanel->m_saveFunction = [settingTextField, settingEnabledCheckBox, setSettingValueFunction, clearSettingValueFunction]() {
		if(settingEnabledCheckBox->GetValue()) {
			setSettingValueFunction(settingTextField->GetValue());
		}
		else {
			clearSettingValueFunction();
		}
	};

	settingPanel->m_discardFunction = [settingTextField, settingEnabledCheckBox, getSettingValueFunction]() {
		std::optional<std::string> setting(getSettingValueFunction());

		settingTextField->ChangeValue(wxString::FromUTF8(setting.value_or("")));
		settingEnabledCheckBox->SetValue(setting.has_value());
		WXUtilities::setTextControlEnabled(settingTextField, setting.has_value());
	};

	settingPanel->m_resetFunction = [settingTextField, settingEnabledCheckBox, defaultSetting]() {
		settingTextField->ChangeValue(wxString::FromUTF8(defaultSetting.value_or("")));
		settingEnabledCheckBox->SetValue(defaultSetting.has_value());
		WXUtilities::setTextControlEnabled(settingTextField, defaultSetting.has_value());
	};

	settingPanel->m_setEditableFunction = [settingTextField, settingEnabledCheckBox](bool editable) {
		settingTextField->SetEditable(editable);

		if(editable) {
			settingEnabledCheckBox->Enable();
		}
		else {
			settingEnabledCheckBox->Disable();
		}
	};

	wxBoxSizer * settingBoxSizer = new wxBoxSizer(wxVERTICAL);
	settingBoxSizer->Add(settingEnabledCheckBox, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingBoxSizer->Add(settingTextField, 1, wxEXPAND | wxHORIZONTAL, 2);
	settingPanel->SetSizer(settingBoxSizer);

	if(parentSizer != nullptr) {
		parentSizer->Add(settingPanel, 1, wxEXPAND | wxALL, 5);
	}

	return settingPanel;
}

template <typename E>
SettingPanel * SettingPanel::createEnumSettingPanel(E & setting, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, const std::vector<E> & disabledEnumValues = {}) {
	return createEnumSettingPanel<E>([&setting]() -> E & { return setting; }, [&setting](E newSetting) { setting = newSetting; }, defaultSetting, name, parent, parentSizer, disabledEnumValues);
}

template <typename E>
SettingPanel * SettingPanel::createEnumSettingPanel(std::function<E()> getSettingValueFunction, std::function<void(E)> setSettingValueFunction, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, const std::vector<E> & disabledEnumValues = {}) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	constexpr auto enumValues = magic_enum::enum_values<E>();
	std::vector<E> enabledEnumValues;

	std::copy_if(enumValues.cbegin(), enumValues.cend(), std::back_inserter(enabledEnumValues), [&disabledEnumValues, &enumValues](const E enumValue) {
		return std::find(disabledEnumValues.cbegin(), disabledEnumValues.cend(), enumValue) == disabledEnumValues.cend();
	});

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxComboBox * settingComboBox = new wxComboBox(settingPanel, wxID_ANY, Utilities::toCapitalCase(magic_enum::enum_name(getSettingValueFunction())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<E>(disabledEnumValues), 0, wxDefaultValidator, name);
	settingComboBox->SetEditable(false);
	settingComboBox->Bind(wxEVT_COMBOBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingComboBox]() {
		return settingComboBox->GetValue();
	};

	settingPanel->m_defaultValidatorFunction = [enumValues, settingComboBox]() {
		int selection = settingComboBox->GetSelection();

		return selection != wxNOT_FOUND && selection < enumValues.size();
	};

	settingPanel->m_saveFunction = [enumValues, settingComboBox, setSettingValueFunction]() {
		setSettingValueFunction(enumValues[settingComboBox->GetSelection()]);
	};

	settingPanel->m_discardFunction = [enumValues, settingComboBox, getSettingValueFunction]() {
		auto enumIterator = std::find(enumValues.cbegin(), enumValues.cend(), getSettingValueFunction());

		if(enumIterator == enumValues.cend()) {
			return;
		}

		settingComboBox->SetSelection(enumIterator - enumValues.cbegin());
	};

	settingPanel->m_resetFunction = [enumValues, settingComboBox, defaultSetting]() {
		auto enumIterator = std::find(enumValues.cbegin(), enumValues.cend(), defaultSetting);

		if(enumIterator == enumValues.cend()) {
			return;
		}

		settingComboBox->SetSelection(enumIterator - enumValues.cbegin());
	};

	settingPanel->m_setEditableFunction = [settingComboBox](bool editable) {
		settingComboBox->SetEditable(editable);
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

template <typename E>
SettingPanel * SettingPanel::createEnumMultiChoiceSettingPanel(std::vector<E> & setting, const std::string & name, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer) {
	return createStringMultiChoiceSettingPanel(
		[&setting]() -> const std::vector<E> & {
			return setting;
		},
		[&setting](E entry) {
			return std::find_if(setting.cbegin(), setting.cend(), [entry](E currentEntry) {
				return entry == currentEntry;
			}) != setting.cend();
		},
		[&setting](E entry) {
			if(std::find_if(setting.cbegin(), setting.cend(), [entry](E currentEntry) {
				return entry == currentEntry;
			}) != setting.cend()) {
				return false;
			}

			setting.push_back(entry);

			return true;
		},
		[&setting](E entry) {
			auto entryIterator = std::find_if(setting.cbegin(), setting.cend(), [entry](E currentEntry) {
				return entry == currentEntry;
			});

			if(entryIterator == setting.cend()) {
				return false;
			}

			setting.erase(entryIterator);

			return true;
		},
		name,
		parent,
		minimumNumberOfSelectedItems,
		parentSizer
	);
}

template <typename E>
SettingPanel * SettingPanel::createEnumMultiChoiceSettingPanel(std::function<const std::vector<E> &()> getSettingValueFunction, std::function<bool(E)> hasSettingEntryFunction, std::function<bool(E)> addSettingEntryFunction, std::function<bool(E)> removeSettingEntryFunction, const std::string & name, wxWindow * parent, size_t minimumNumberOfSelectedItems, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxCheckBox * checkBox = nullptr;
	std::vector<wxCheckBox *> checkBoxes;
	checkBoxes.reserve(magic_enum::enum_count<E>());

	int border = 5;

	wxWrapSizer * settingSizer = new wxWrapSizer(wxHORIZONTAL);

	for(E e : magic_enum::enum_values<E>()) {
		std::string choiceName(magic_enum::enum_name(e));
		checkBox = new wxCheckBox(settingPanel, wxID_ANY, choiceName, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator, choiceName);
		checkBox->SetFont(checkBox->GetFont().MakeBold());
		checkBox->SetValue(hasSettingEntryFunction(e));
		checkBox->Bind(wxEVT_CHECKBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

		checkBoxes.push_back(checkBox);

		settingSizer->Add(checkBox, 1, wxEXPAND | wxHORIZONTAL, border);
	}

	settingPanel->m_getValueFunction = [checkBoxes]() {
		std::stringstream valueStream;

		for(size_t i = 0; i < magic_enum::enum_count<E>(); i++) {
			if(!checkBoxes[i]->GetValue()) {
				continue;
			}

			if(valueStream.tellp() != 0) {
				valueStream << ", ";
			}

			valueStream << magic_enum::enum_name(magic_enum::enum_value<E>(i));
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

	settingPanel->m_saveFunction = [checkBoxes, hasSettingEntryFunction, addSettingEntryFunction, removeSettingEntryFunction]() {
		for(size_t i = 0; i < magic_enum::enum_count<E>(); i++) {
			E e = magic_enum::enum_value<E>(i);
			bool hasEntry = hasSettingEntryFunction(e);

			if(checkBoxes[i]->GetValue()) {
				if(!hasEntry) {
					addSettingEntryFunction(e);
				}
			}
			else {
				if(hasEntry) {
					removeSettingEntryFunction(e);
				}
			}
		}
	};

	settingPanel->m_discardFunction = [checkBoxes, hasSettingEntryFunction]() {
		for(size_t i = 0; i < magic_enum::enum_count<E>(); i++) {
			checkBoxes[i]->SetValue(hasSettingEntryFunction(magic_enum::enum_value<E>(i)));
		}
	};

	settingPanel->m_resetFunction = [checkBoxes]() {
		for(size_t i = 0; i < magic_enum::enum_count<E>(); i++) {
			checkBoxes[i]->SetValue(false);
		}
	};

	settingPanel->m_setEditableFunction = [checkBoxes](bool editable) {
		for(size_t i = 0; i < magic_enum::enum_count<E>(); i++) {
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

#endif // _SETTING_PANEL_H_
