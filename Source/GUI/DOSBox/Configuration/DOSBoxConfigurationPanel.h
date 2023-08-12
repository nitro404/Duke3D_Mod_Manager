#ifndef _DOSBOX_CONFIGURATION_PANEL_H_
#define _DOSBOX_CONFIGURATION_PANEL_H_

#include "DOSBox/Configuration/DOSBoxConfiguration.h"

#include <Signal/SignalConnectionGroup.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class wxDataViewCtrl;
class wxPropertyGrid;
class wxPropertyGridEvent;

class DOSBoxConfigurationPanel final : public wxPanel {
public:
	DOSBoxConfigurationPanel(std::shared_ptr<DOSBoxConfiguration> dosboxConfiguration, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~DOSBoxConfigurationPanel();

	bool isEnabled() const;
	void enable();
	void disable();
	void setEnabled(bool enabled);
	bool isModified() const;
	std::string getPanelName() const;
	std::shared_ptr<DOSBoxConfiguration> getDOSBoxConfiguration() const;
	bool isValid() const;
	bool save();
	void discard();

	bool selectSection(size_t sectionIndex);
	bool selectSection(std::shared_ptr<DOSBoxConfiguration::Section> section);
	void clearSelectedSection();
	void update();

private:
	void updateButtons();
	void onSectionSelected(wxCommandEvent & event);
	void onConfigurationSectionPropertyGridChanged(wxPropertyGridEvent & event);
	void onAddSectionButtonPressed(wxCommandEvent & event);
	void onRenameSectionButtonPressed(wxCommandEvent & event);
	void onRemoveSectionButtonPressed(wxCommandEvent & event);
	void onAddEntryButtonPressed(wxCommandEvent & event);
	void onEditEntryButtonPressed(wxCommandEvent & event);
	void onRenameEntryButtonPressed(wxCommandEvent & event);
	void onRemoveEntryButtonPressed(wxCommandEvent & event);
	void onDiscardConfigurationButtonPressed(wxCommandEvent & event);
	void onSaveConfigurationButtonPressed(wxCommandEvent & event);
	void onConfigurationModified(DOSBoxConfiguration & dosboxConfiguration);
	void onConfigurationSectionNameChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::string oldSectionName);
	void onConfigurationSectionAdded(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex);
	void onConfigurationSectionReplaced(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section> oldSection);
	void onConfigurationSectionInserted(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex);
	void onConfigurationSectionRemoved(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex);
	void onConfigurationSectionsCleared(DOSBoxConfiguration & dosboxConfiguration);
	void onConfigurationSectionEntryNameChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex, std::string oldEntryName);
	void onConfigurationSectionEntryValueChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex, std::string oldEntryValue);
	void onConfigurationSectionEntryAdded(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex);
	void onConfigurationSectionEntryReplaced(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> oldEntry);
	void onConfigurationSectionEntryInserted(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex);
	void onConfigurationSectionEntryRemoved(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex);
	void onConfigurationSectionEntriesCleared(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex);

	std::shared_ptr<DOSBoxConfiguration> m_dosboxConfiguration;
	std::shared_ptr<DOSBoxConfiguration> m_lastSavedDOSBoxConfiguration;
	std::shared_ptr<DOSBoxConfiguration::Section> m_selectedSection;
	SignalConnectionGroup m_dosboxConfigurationConnections;
	bool m_enabled;

	wxListBox * m_sectionListBox;
	wxPropertyGrid * m_configurationSectionPropertyGrid;
	wxButton * m_addSectionButton;
	wxButton * m_renameSectionButton;
	wxButton * m_removeSectionButton;
	wxButton * m_addEntryButton;
	wxButton * m_editEntryButton;
	wxButton * m_renameEntryButton;
	wxButton * m_removeEntryButton;
	wxButton * m_discardConfigurationButton;
	wxButton * m_saveConfigurationButton;
	std::array<wxButton *, 9> m_actionButtons;

	DOSBoxConfigurationPanel(const DOSBoxConfigurationPanel &) = delete;
	const DOSBoxConfigurationPanel & operator = (const DOSBoxConfigurationPanel &) = delete;
};

#endif // _DOSBOX_CONFIGURATION_PANEL_H_
