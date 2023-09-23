#ifndef _MOD_DEPENDENCIES_PANEL_H_
#define _MOD_DEPENDENCIES_PANEL_H_

#include <Signal/SignalConnectionGroup.h>

#include <boost/signals2.hpp>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>
#include <vector>

class ModCollection;
class ModVersionType;
class ModDependencyPanel;

class ModDependenciesPanel final : public wxPanel {
public:
	ModDependenciesPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModDependenciesPanel();

	void setModVersionType(std::shared_ptr<ModVersionType> modVersionType);

	boost::signals2::signal<void (std::string /* modID */, std::string /* modVersion */, std::string /* modVersionType */)> modVersionTypeSelectionRequested;

private:
	void onModVersionTypeSelectionRequested(const std::string & modID, const std::string & modVersion, const std::string & modVersionType);

	std::shared_ptr<ModVersionType> m_modVersionType;
	std::shared_ptr<ModCollection> m_mods;

	std::vector<ModDependencyPanel *> m_dependencyPanels;
	wxFlexGridSizer * m_dependenciesPanelSizer;
	SignalConnectionGroup m_modDependencyPanelConnections;

	ModDependenciesPanel(const ModDependenciesPanel &) = delete;
	const ModDependenciesPanel & operator = (const ModDependenciesPanel &) = delete;
};

#endif // _MOD_DEPENDENCIES_PANEL_H_
