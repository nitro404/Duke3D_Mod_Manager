#ifndef _SETTING_PANEL_H_
#define _SETTING_PANEL_H_

#include <Utilities/Utilities.h>

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

class SettingPanel final : public wxPanel {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void settingModified(SettingPanel * settingPanel) = 0;
	};

	virtual ~SettingPanel();

	const std::string & getName() const;
	std::string getValue() const;
	bool isModified() const;
	bool isValid() const;
	bool save();
	void discard();

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	static SettingPanel * createBooleanSettingPanel(bool & setting, bool defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	template <typename T>
	static SettingPanel * createIntegerSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue = std::numeric_limits<T>::min(), size_t maxValue = std::numeric_limits<T>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename T>
	static SettingPanel * createChronoSettingPanel(T & setting, const T & defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	static SettingPanel * createStringSettingPanel(std::string & setting, const std::string & defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minLength = 0, size_t maxLength = std::numeric_limits<size_t>::max(), std::function<bool(const SettingPanel *)> customValidatorFunction = nullptr);
	template <typename E>
	static SettingPanel * createEnumSettingPanel(E & setting, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer);
	static SettingPanel * createStringChoiceSettingPanel(std::string & setting, const std::string & defaultSetting, const std::string & name, const std::vector<std::string> & choices, wxWindow * parent, wxSizer * parentSizer);

private:
	SettingPanel(const std::string & name, wxWindow * parent);

	void setModified(bool modified);
	void notifySettingModified();

	std::string m_name;
	bool m_modified;
	std::function<void(wxCommandEvent &)> m_changedFunction;
	std::function<std::string()> m_getValueFunction;
	std::function<bool()> m_defaultValidatorFunction;
	std::function<bool(const SettingPanel *)> m_customValidatorFunction;
	std::function<bool()> m_saveFunction;
	std::function<void()> m_discardFunction;
	std::vector<Listener *> m_listeners;
};

template <typename T>
static SettingPanel * SettingPanel::createIntegerSettingPanel(T & setting, T defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, size_t minValue, size_t maxValue, std::function<bool(const SettingPanel *)> customValidatorFunction) {
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
	wxTextCtrl * settingTextField = new wxTextCtrl(settingPanel, wxID_ANY, std::to_string(setting), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, name);
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

	settingPanel->m_saveFunction = [settingPanel, settingTextField, minValue, maxValue, &setting]() {
		bool error = false;

		if(std::is_signed<T>()) {
			int64_t value = Utilities::parseLong(settingTextField->GetValue(), &error);

			if(error ||
			   value < std::numeric_limits<T>::min() ||
			   value > std::numeric_limits<T>::max() ||
			   value < minValue ||
			   value > maxValue) {
				return false;
			}

			setting = static_cast<T>(value);
		}
		else {
			uint64_t value = Utilities::parseUnsignedLong(settingTextField->GetValue(), &error);

			if(error ||
			   value > std::numeric_limits<T>::max() ||
			   value < minValue ||
			   value > maxValue) {
				return false;
			}

			setting = static_cast<T>(value);
		}

		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingTextField, &setting]() {
		settingTextField->SetValue(std::to_string(setting));
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

template <typename T>
static SettingPanel * SettingPanel::createChronoSettingPanel(T & setting, const T & defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer, std::function<bool(const SettingPanel *)> customValidatorFunction) {
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

	settingPanel->m_defaultValidatorFunction = [settingPanel, settingTextField]() {
		bool error = false;
		int64_t value = Utilities::parseLong(settingTextField->GetValue(), &error);

		return !error &&
			   value >= 0;
	};

	settingPanel->m_saveFunction = [settingPanel, settingTextField, &setting]() {
		bool error = false;
		int64_t value = Utilities::parseLong(settingTextField->GetValue(), &error);

		if(error || value < 0) {
			return false;
		}

		setting = T(value);
		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingTextField, &setting]() {
		settingTextField->SetValue(std::to_string(setting.count()));
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

template <typename E>
static SettingPanel * SettingPanel::createEnumSettingPanel(E & setting, E defaultSetting, const std::string & name, wxWindow * parent, wxSizer * parentSizer) {
	if(parent == nullptr) {
		return nullptr;
	}

	SettingPanel * settingPanel = new SettingPanel(name, parent);

	settingPanel->m_changedFunction = [settingPanel](wxCommandEvent & event) {
		settingPanel->setModified(true);
	};

	wxStaticText * settingLabel = new wxStaticText(settingPanel, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	settingLabel->SetFont(settingLabel->GetFont().MakeBold());
	wxComboBox * settingComboBox = new wxComboBox(settingPanel, wxID_ANY, std::string(magic_enum::enum_name(setting)), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<E>(), 0, wxDefaultValidator, name);
	settingComboBox->SetEditable(false);
	settingComboBox->Bind(wxEVT_COMBOBOX, settingPanel->m_changedFunction, wxID_ANY, wxID_ANY);

	settingPanel->m_getValueFunction = [settingComboBox]() {
		return settingComboBox->GetValue();
	};

	settingPanel->m_defaultValidatorFunction = [settingComboBox]() {
		return magic_enum::enum_cast<E>(std::string(settingComboBox->GetValue())).has_value();
	};

	settingPanel->m_saveFunction = [settingPanel, settingComboBox, &setting]() {
		std::optional<E> optionalEnumValue(magic_enum::enum_cast<E>(std::string(settingComboBox->GetValue())));

		if(!optionalEnumValue.has_value()) {
			return false;
		}

		setting = optionalEnumValue.value();
		settingPanel->setModified(false);

		return true;
	};

	settingPanel->m_discardFunction = [settingPanel, settingComboBox, &setting]() {
		std::optional<uint64_t> optionalEnumIndex = magic_enum::enum_index(setting);

		if(optionalEnumIndex.has_value()) {
			settingComboBox->SetSelection(optionalEnumIndex.value());
			settingPanel->setModified(false);
		}
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

#endif // _SETTING_PANEL_H_
