use std::ops::{Add, AddAssign, Div, Mul, Neg, Sub};

#[derive(Clone, Copy, Default)]
pub(crate) struct Vec3 {
    pub(crate) x: f32,
    pub(crate) y: f32,
    pub(crate) z: f32,
}

impl Vec3 {
    pub(crate) const ZERO: Self = Self::new(0.0, 0.0, 0.0);

    pub(crate) const fn new(x: f32, y: f32, z: f32) -> Self {
        Self { x, y, z }
    }

    pub(crate) fn from_array(data: [f32; 3]) -> Self {
        Self::new(data[0], data[1], data[2])
    }

    pub(crate) fn length(self) -> f32 {
        self.dot(self).sqrt()
    }

    pub(crate) fn dot(self, other: Self) -> f32 {
        self.x * other.x + self.y * other.y + self.z * other.z
    }

    pub(crate) fn cross(self, other: Self) -> Self {
        Self::new(
            self.y * other.z - self.z * other.y,
            self.z * other.x - self.x * other.z,
            self.x * other.y - self.y * other.x,
        )
    }

    pub(crate) fn normalize(self) -> Self {
        let len = self.length();
        if len > 0.0 { self / len } else { Self::ZERO }
    }

    pub(crate) fn clamp_magnitude(self, max_mag: f32) -> Self {
        let len = self.length();
        if len <= max_mag {
            self
        } else {
            self * (max_mag / len)
        }
    }
}

impl Add for Vec3 {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self::new(self.x + rhs.x, self.y + rhs.y, self.z + rhs.z)
    }
}

impl AddAssign for Vec3 {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}

impl Sub for Vec3 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        Self::new(self.x - rhs.x, self.y - rhs.y, self.z - rhs.z)
    }
}

impl Neg for Vec3 {
    type Output = Self;

    fn neg(self) -> Self::Output {
        Self::new(-self.x, -self.y, -self.z)
    }
}

impl Mul<f32> for Vec3 {
    type Output = Self;

    fn mul(self, rhs: f32) -> Self::Output {
        Self::new(self.x * rhs, self.y * rhs, self.z * rhs)
    }
}

impl Div<f32> for Vec3 {
    type Output = Self;

    fn div(self, rhs: f32) -> Self::Output {
        Self::new(self.x / rhs, self.y / rhs, self.z / rhs)
    }
}

#[derive(Clone, Copy)]
pub(crate) struct Quat {
    pub(crate) w: f32,
    pub(crate) x: f32,
    pub(crate) y: f32,
    pub(crate) z: f32,
}

impl Quat {
    pub(crate) fn from_array(data: [f32; 4]) -> Self {
        Self {
            w: data[0],
            x: data[1],
            y: data[2],
            z: data[3],
        }
    }

    pub(crate) fn from_basis(right: Vec3, up: Vec3, back: Vec3) -> Self {
        let m00 = right.x;
        let m01 = up.x;
        let m02 = back.x;
        let m10 = right.y;
        let m11 = up.y;
        let m12 = back.y;
        let m20 = right.z;
        let m21 = up.z;
        let m22 = back.z;
        let trace = m00 + m11 + m22;

        let quat = if trace > 0.0 {
            let s = (trace + 1.0).sqrt() * 2.0;
            Self {
                w: 0.25 * s,
                x: (m21 - m12) / s,
                y: (m02 - m20) / s,
                z: (m10 - m01) / s,
            }
        } else if m00 > m11 && m00 > m22 {
            let s = (1.0 + m00 - m11 - m22).sqrt() * 2.0;
            Self {
                w: (m21 - m12) / s,
                x: 0.25 * s,
                y: (m01 + m10) / s,
                z: (m02 + m20) / s,
            }
        } else if m11 > m22 {
            let s = (1.0 + m11 - m00 - m22).sqrt() * 2.0;
            Self {
                w: (m02 - m20) / s,
                x: (m01 + m10) / s,
                y: 0.25 * s,
                z: (m12 + m21) / s,
            }
        } else {
            let s = (1.0 + m22 - m00 - m11).sqrt() * 2.0;
            Self {
                w: (m10 - m01) / s,
                x: (m02 + m20) / s,
                y: (m12 + m21) / s,
                z: 0.25 * s,
            }
        };

        quat.normalize()
    }

    pub(crate) fn conjugate(self) -> Self {
        Self {
            w: self.w,
            x: -self.x,
            y: -self.y,
            z: -self.z,
        }
    }

    pub(crate) fn normalize(self) -> Self {
        let len = (self.w * self.w + self.x * self.x + self.y * self.y + self.z * self.z).sqrt();
        if len > 0.0 {
            Self {
                w: self.w / len,
                x: self.x / len,
                y: self.y / len,
                z: self.z / len,
            }
        } else {
            Self {
                w: 1.0,
                x: 0.0,
                y: 0.0,
                z: 0.0,
            }
        }
    }

    pub(crate) fn axis_angle_vector(self) -> Vec3 {
        let q = self.normalize();
        let angle = 2.0 * q.w.clamp(-1.0, 1.0).acos();
        let sin_half = (1.0 - q.w * q.w).max(0.0).sqrt();

        if sin_half < 0.0001 {
            Vec3::ZERO
        } else {
            Vec3::new(q.x / sin_half, q.y / sin_half, q.z / sin_half) * angle
        }
    }
}

impl Mul for Quat {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        Self {
            w: self.w * rhs.w - self.x * rhs.x - self.y * rhs.y - self.z * rhs.z,
            x: self.w * rhs.x + self.x * rhs.w + self.y * rhs.z - self.z * rhs.y,
            y: self.w * rhs.y - self.x * rhs.z + self.y * rhs.w + self.z * rhs.x,
            z: self.w * rhs.z + self.x * rhs.y - self.y * rhs.x + self.z * rhs.w,
        }
    }
}

impl Neg for Quat {
    type Output = Self;

    fn neg(self) -> Self::Output {
        Self {
            w: -self.w,
            x: -self.x,
            y: -self.y,
            z: -self.z,
        }
    }
}

pub(crate) fn solve_4x4<const N: usize>(
    mut matrix: [[f32; N]; N],
    mut rhs: [f32; N],
) -> Option<[f32; N]> {
    for pivot in 0..N {
        let mut pivot_row = pivot;
        let mut pivot_value = matrix[pivot][pivot].abs();

        for row in (pivot + 1)..N {
            let value = matrix[row][pivot].abs();
            if value > pivot_value {
                pivot_row = row;
                pivot_value = value;
            }
        }

        if pivot_value <= f32::EPSILON {
            return None;
        }

        if pivot_row != pivot {
            matrix.swap(pivot, pivot_row);
            rhs.swap(pivot, pivot_row);
        }

        let divisor = matrix[pivot][pivot];
        for col in pivot..N {
            matrix[pivot][col] /= divisor;
        }
        rhs[pivot] /= divisor;

        for row in 0..N {
            if row == pivot {
                continue;
            }

            let factor = matrix[row][pivot];
            for col in pivot..N {
                matrix[row][col] -= factor * matrix[pivot][col];
            }
            rhs[row] -= factor * rhs[pivot];
        }
    }

    Some(rhs)
}
