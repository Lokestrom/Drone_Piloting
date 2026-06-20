#[path = "API.rs"]
mod api;
mod math;

use std::ffi::CStr;
use std::os::raw::c_char;
use std::ptr;

use api::{
    ButtonState, CommandBuffer, DroneState, EngineCommand, InputType, SettingsBuffer, UserInput,
};
use math::{Quat, Vec3, solve_4x4};

const ENGINE_COUNT: usize = 4;
const SETTING_COUNT: usize = 11;
const GRAVITY: f32 = 9.81;
const MASS: f32 = 1.0;
const LAMBDA: f32 = 0.000001;

const ENGINE_POSITIONS: [Vec3; ENGINE_COUNT] = [
    Vec3::new(0.707, 0.0, 0.707),
    Vec3::new(0.707, 0.0, -0.707),
    Vec3::new(-0.707, 0.0, 0.707),
    Vec3::new(-0.707, 0.0, -0.707),
];
const ENGINE_FORCE_DIRECTION: Vec3 = Vec3::new(0.0, 1.0, 0.0);
const ENGINE_MAX_THRUST: f32 = 10.0;

static mut POS_KP: f32 = 1.0;
static mut POS_KI: f32 = 0.0;
static mut VEL_KP: f32 = 2.0;
static mut VEL_KI: f32 = 0.05;
static mut VEL_KD: f32 = 1.5;
static mut ATT_KP: f32 = 8.0;
static mut ATT_KI: f32 = 0.5;
static mut RATE_KP: f32 = 2.0;
static mut RATE_KD: f32 = 3.0;
static mut MOVE_SPEED: f32 = 20.0;
static mut ALTITUDE_SPEED: f32 = 10.0;

#[derive(Clone, Copy)]
struct InputHandles {
    forward: *const f32,
    left: *const f32,
    right: *const f32,
    up: *const ButtonState,
    down: *const ButtonState,
}

impl InputHandles {
    const fn empty() -> Self {
        Self {
            forward: ptr::null(),
            left: ptr::null(),
            right: ptr::null(),
            up: ptr::null(),
            down: ptr::null(),
        }
    }
}

#[derive(Clone, Copy)]
struct Pid {
    integral: Vec3,
}

impl Pid {
    const fn new() -> Self {
        Self {
            integral: Vec3::ZERO,
        }
    }

    fn update(&mut self, error: Vec3, kp: f32, ki: f32, dt: f32) -> Vec3 {
        self.integral += error * dt;
        error * kp + self.integral * ki
    }
}

struct ControllerState {
    position_pid: Pid,
    velocity_pid: Pid,
    attitude_pid: Pid,
}

impl ControllerState {
    const fn new() -> Self {
        Self {
            position_pid: Pid::new(),
            velocity_pid: Pid::new(),
            attitude_pid: Pid::new(),
        }
    }
}

static mut INPUTS: InputHandles = InputHandles::empty();
static mut CONTROLLER: ControllerState = ControllerState::new();
static mut INITIALIZED: bool = false;
static mut TARGET_POSITION: Vec3 = Vec3::ZERO;
static mut ENGINE_COMMANDS: [EngineCommand; ENGINE_COUNT] = [
    EngineCommand {
        engine_id: 0,
        thrust: 0.0,
    },
    EngineCommand {
        engine_id: 1,
        thrust: 0.0,
    },
    EngineCommand {
        engine_id: 2,
        thrust: 0.0,
    },
    EngineCommand {
        engine_id: 3,
        thrust: 0.0,
    },
];

static mut SETTING_NAMES: [*const c_char; SETTING_COUNT] = [ptr::null(); SETTING_COUNT];
static mut SETTING_VALUES: [*mut f32; SETTING_COUNT] = [ptr::null_mut(); SETTING_COUNT];
static mut SETTINGS: SettingsBuffer = SettingsBuffer {
    names: ptr::null(),
    values: ptr::null_mut(),
    count: 0,
};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn setup(_drone_path: *const c_char, input: *const UserInput) {
    unsafe {
        INPUTS = InputHandles::empty();
        CONTROLLER = ControllerState::new();
        INITIALIZED = false;
        initialize_settings();

        if input.is_null() {
            return;
        }

        let input = &*input;
        if input.names.is_null() {
            return;
        }

        let mut button_index = 0_usize;
        let mut axis_index = 0_usize;

        for index in 0..input.size as usize {
            let name_ptr = *input.names.add(index);
            if name_ptr.is_null() {
                continue;
            }

            let Ok(name) = CStr::from_ptr(name_ptr).to_str() else {
                continue;
            };

            if !input.types.is_null() && *input.types.add(index) == InputType::Button {
                if !input.button_pressed.is_null() {
                    match name {
                        "Up" => INPUTS.up = input.button_pressed.add(button_index),
                        "Down" => INPUTS.down = input.button_pressed.add(button_index),
                        _ => {}
                    }
                }
                button_index += 1;
            } else {
                if !input.axis_values.is_null() {
                    match name {
                        "Forward" => INPUTS.forward = input.axis_values.add(axis_index),
                        "Left" => INPUTS.left = input.axis_values.add(axis_index),
                        "Right" => INPUTS.right = input.axis_values.add(axis_index),
                        _ => {}
                    }
                }
                axis_index += 1;
            }
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn update(
    state: *const DroneState,
    dt: f32,
    active: bool,
    out_commands: *mut CommandBuffer,
) {
    unsafe {
        if state.is_null() || dt <= 0.0 || out_commands.is_null() {
            return;
        }

        let state = &*state;
        let position = Vec3::from_array(state.position);
        let velocity = Vec3::from_array(state.velocity);
        let orientation = Quat::from_array(state.orientation).normalize();
        let angular_velocity = Vec3::from_array(state.angular_velocity);

        if !INITIALIZED {
            TARGET_POSITION = position;
            INITIALIZED = true;
        }

        if active {
            update_target_position(dt);
        }

        let controller = &mut *ptr::addr_of_mut!(CONTROLLER);

        let position_error = TARGET_POSITION - position;
        let desired_velocity = controller
            .position_pid
            .update(position_error, POS_KP, POS_KI, dt);
        let desired_velocity = desired_velocity.clamp_magnitude(MOVE_SPEED);
        let velocity_error = desired_velocity - velocity;

        controller.velocity_pid.integral += velocity_error * dt;
        let mut desired_acceleration =
            velocity_error * VEL_KP + controller.velocity_pid.integral * VEL_KI - velocity * VEL_KD;

        desired_acceleration += Vec3::new(0.0, GRAVITY, 0.0);
        let vertical = desired_acceleration.y;
        desired_acceleration = desired_acceleration.clamp_magnitude(20.0);
        desired_acceleration.y = vertical;

        let up = desired_acceleration.normalize();
        let world_forward = Vec3::new(0.0, 0.0, -1.0);
        let right = world_forward.cross(up).normalize();
        let forward = up.cross(right).normalize();
        let target_orientation = Quat::from_basis(right, up, -forward);

        let mut q_error = target_orientation * orientation.conjugate();
        if q_error.w < 0.0 {
            q_error = -q_error;
        }

        let attitude_error = q_error.axis_angle_vector();
        let desired_angular_rate =
            controller
                .attitude_pid
                .update(attitude_error, ATT_KP, ATT_KI, dt);
        let angular_rate_error = desired_angular_rate - angular_velocity;

        let desired_torque = angular_rate_error * RATE_KP - angular_velocity * RATE_KD;
        let desired_force = desired_acceleration * MASS;
        solve_system(desired_force, desired_torque);

        (*out_commands).commands = ptr::addr_of_mut!(ENGINE_COMMANDS).cast::<EngineCommand>();
        (*out_commands).count = ENGINE_COUNT as u64;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn getTargetPosition(out_position: *mut f32) {
    unsafe {
        if out_position.is_null() {
            return;
        }

        *out_position.add(0) = TARGET_POSITION.x;
        *out_position.add(1) = TARGET_POSITION.y;
        *out_position.add(2) = TARGET_POSITION.z;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn getSettings() -> *mut SettingsBuffer {
    unsafe {
        initialize_settings();
        ptr::addr_of_mut!(SETTINGS)
    }
}

unsafe fn initialize_settings() {
    unsafe {
        SETTING_NAMES = [
            c_str_ptr(b"POS_KP\0"),
            c_str_ptr(b"POS_KI\0"),
            c_str_ptr(b"VEL_KP\0"),
            c_str_ptr(b"VEL_KI\0"),
            c_str_ptr(b"VEL_KD\0"),
            c_str_ptr(b"ATT_KP\0"),
            c_str_ptr(b"ATT_KI\0"),
            c_str_ptr(b"RATE_KP\0"),
            c_str_ptr(b"RATE_KD\0"),
            c_str_ptr(b"MOVE_SPEED\0"),
            c_str_ptr(b"ALTITUDE_SPEED\0"),
        ];

        SETTING_VALUES = [
            ptr::addr_of_mut!(POS_KP),
            ptr::addr_of_mut!(POS_KI),
            ptr::addr_of_mut!(VEL_KP),
            ptr::addr_of_mut!(VEL_KI),
            ptr::addr_of_mut!(VEL_KD),
            ptr::addr_of_mut!(ATT_KP),
            ptr::addr_of_mut!(ATT_KI),
            ptr::addr_of_mut!(RATE_KP),
            ptr::addr_of_mut!(RATE_KD),
            ptr::addr_of_mut!(MOVE_SPEED),
            ptr::addr_of_mut!(ALTITUDE_SPEED),
        ];

        SETTINGS = SettingsBuffer {
            names: ptr::addr_of!(SETTING_NAMES).cast::<*const c_char>(),
            values: ptr::addr_of_mut!(SETTING_VALUES).cast::<*mut f32>(),
            count: SETTING_COUNT as u64,
        };
    }
}

const fn c_str_ptr(value: &'static [u8]) -> *const c_char {
    value.as_ptr().cast::<c_char>()
}

unsafe fn update_target_position(dt: f32) {
    unsafe {
        let mut movement = Vec3::ZERO;
        movement.z += axis_value(INPUTS.forward);
        movement.x += axis_value(INPUTS.left) - axis_value(INPUTS.right);

        if movement.length() > 0.0 {
            movement = movement.normalize();
        }

        movement = movement * MOVE_SPEED;
        TARGET_POSITION += movement * dt;

        if button_is_down(INPUTS.up) {
            TARGET_POSITION.y += ALTITUDE_SPEED * dt;
        }

        if button_is_down(INPUTS.down) {
            TARGET_POSITION.y -= ALTITUDE_SPEED * dt;
        }

        if TARGET_POSITION.y < -10.0 {
            TARGET_POSITION.y = -10.0;
        }
    }
}

unsafe fn solve_system(desired_force: Vec3, desired_torque: Vec3) {
    unsafe {
        let mut a = [[0.0_f32; ENGINE_COUNT]; 6];

        for engine in 0..ENGINE_COUNT {
            let torque_axis = ENGINE_POSITIONS[engine].cross(ENGINE_FORCE_DIRECTION);

            a[0][engine] = ENGINE_FORCE_DIRECTION.x;
            a[1][engine] = ENGINE_FORCE_DIRECTION.y;
            a[2][engine] = ENGINE_FORCE_DIRECTION.z;
            a[3][engine] = torque_axis.x;
            a[4][engine] = torque_axis.y;
            a[5][engine] = torque_axis.z;
        }

        let b = [
            desired_force.x,
            desired_force.y,
            desired_force.z,
            desired_torque.x,
            desired_torque.y,
            desired_torque.z,
        ];

        let mut ata = [[0.0_f32; ENGINE_COUNT]; ENGINE_COUNT];
        let mut atb = [0.0_f32; ENGINE_COUNT];

        for row in 0..ENGINE_COUNT {
            for col in 0..ENGINE_COUNT {
                for source_row in 0..6 {
                    ata[row][col] += a[source_row][row] * a[source_row][col];
                }
            }

            ata[row][row] += LAMBDA;

            for source_row in 0..6 {
                atb[row] += a[source_row][row] * b[source_row];
            }
        }

        let thrusts = solve_4x4(ata, atb).unwrap_or([0.0; ENGINE_COUNT]);
        let commands = ptr::addr_of_mut!(ENGINE_COMMANDS).cast::<EngineCommand>();

        for (engine, thrust) in thrusts.iter().enumerate() {
            *commands.add(engine) = EngineCommand {
                engine_id: engine as u64,
                thrust: thrust.clamp(0.0, ENGINE_MAX_THRUST),
            };
        }
    }
}

unsafe fn axis_value(value: *const f32) -> f32 {
    unsafe {
        if value.is_null() {
            0.0
        } else {
            (*value).clamp(-1.0, 1.0)
        }
    }
}

unsafe fn button_is_down(value: *const ButtonState) -> bool {
    unsafe {
        if value.is_null() {
            return false;
        }

        matches!(*value, ButtonState::Pressed | ButtonState::Down)
    }
}
