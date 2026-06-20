# Rust testDrone controller

This is a 1 to 1 port of the PID controller found in [PIDcontroller.hpp](../../../../../src/API/helpers/PIDcontroller.hpp).
The port was done using AI.

## Build and install
From the repository root, run:

```bat
assets\Drones\testDrone\src\rust_example\build_and_install.bat
```

The script builds both configurations and copies the DLLs into the locations
used by the drone loader:

- `assets/Drones/testDrone/Debug/control.dll`
- `assets/Drones/testDrone/Release/control.dll`

The script requires `cargo` to be available in `PATH`.
This has to be build after cmake since they override each other.