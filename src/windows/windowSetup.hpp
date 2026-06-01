#include "../gui/guiApp.hpp"

#include "DroneSelect.hpp"
#include "infoWindow.hpp"
#include "settingsWindow.hpp"
#include "console.hpp"
#include "mapSelect.hpp"

void setupWindows() {
	gui::App::addWindow<InfoWindow>();
	gui::App::addWindow<SettingsWindow>();
	gui::App::addWindow<DroneSelectWindow>();
	gui::App::addWindow<ConsoleWindow>();
	gui::App::addWindow<MapSelectWindow>();
}