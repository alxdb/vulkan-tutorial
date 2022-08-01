use ash::vk;

use device::Device;
use instance::Instance;

pub struct Graphics {
    device: Device,
    instance: Instance,
}

impl Graphics {
    pub fn new() -> Self {
        let entry = ash::Entry::linked();
        unsafe {
            let instance = instance::Instance::new(&entry);
            let device = device::Device::new(&instance);
            Graphics { device, instance }
        }
    }
}

mod instance {
    use super::*;

    pub struct Instance {
        pub handle: ash::Instance,
    }

    impl Instance {
        /// Creates a new [`Instance`].
        ///
        /// # Safety
        ///
        /// If `entry` was created using `ash::Entry::load()`, then `entry` must outlive the return value.
        pub unsafe fn new(entry: &ash::Entry) -> Self {
            let create_info = vk::InstanceCreateInfo::default();
            Instance {
                handle: entry.create_instance(&create_info, None).unwrap(),
            }
        }
    }

    impl Drop for Instance {
        fn drop(&mut self) {
            unsafe {
                self.handle.destroy_instance(None);
            }
        }
    }
}

mod device {
    use super::*;

    pub struct Device {
        physical_device: vk::PhysicalDevice,
        handle: ash::Device,
    }

    impl Device {
        /// Creates a new [`Device`].
        ///
        /// # Safety
        ///
        /// `instance` must live longer than return value.
        pub unsafe fn new(instance: &Instance) -> Self {
            let physical_device = vk::PhysicalDevice::null();
            let create_info = vk::DeviceCreateInfo::default();
            Device {
                physical_device,
                handle: instance
                    .handle
                    .create_device(physical_device, &create_info, None)
                    .unwrap(),
            }
        }
    }

    impl Drop for Device {
        fn drop(&mut self) {
            unsafe {
                self.handle.destroy_device(None);
            }
        }
    }
}
