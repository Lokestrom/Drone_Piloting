#include "../gui/guiApp.hpp"

#include "DroneSelect.hpp"
#include "infoWindow.hpp"
#include "settingsWindow.hpp"
#include "console.hpp"
#include "mapSelect.hpp"
#include "playerMenu.hpp"

void setupWindows() {
	// Remember to add window to the tool bar.

	gui::App::addWindow<InfoWindow>();
	gui::App::addWindow<SettingsWindow>();
	gui::App::addWindow<DroneSelectWindow>();
	gui::App::addWindow<PlayerMenuWindow>();
	gui::App::addWindow<MapSelectWindow>();
	gui::App::addWindow<ConsoleWindow>();
}