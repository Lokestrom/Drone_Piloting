use std::os::raw::c_char;

#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ButtonState {
    Pressed = 0,
    Down = 1,
    Released = 2,
    Up = 3,
}

#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum InputType {
    Button = 0,
    Axis1Way = 1,
    Axis2Way = 2,
}

#[repr(C)]
pub struct UserInput {
    pub size: u64,
    pub names: *const *const c_char,
    pub types: *mut InputType,
    pub button_pressed: *mut ButtonState,
    pub axis_values: *mut f32,
}

#[repr(C)]
pub struct DroneState {
    pub position: [f32; 3],
    pub velocity: [f32; 3],
    pub orientation: [f32; 4],
    pub angular_velocity: [f32; 3],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct EngineCommand {
    pub engine_id: u64,
    pub thrust: f32,
}

#[repr(C)]
pub struct CommandBuffer {
    pub commands: *mut EngineCommand,
    pub count: u64,
}

#[repr(C)]
pub struct SettingsBuffer {
    pub names: *const *const c_char,
    pub values: *mut *mut f32,
    pub count: u64,
}
