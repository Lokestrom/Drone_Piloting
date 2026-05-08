#include "../gui/guiApp.hpp"

#include "DroneSelect.hpp"
#include "infoWindow.hpp"
#include "settingsWindow.hpp"

void setupWindows() {
	gui::App::addWindow<InfoWindow>();
	gui::App::addWindow<SettingsWindow>();
	gui::App::addWindow<DroneSelectWindow>();
}