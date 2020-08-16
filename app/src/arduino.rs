#![allow(dead_code)]

use numtoa::NumToA;

#[link(wasm_import_module = "arduino")]
extern "C" {
    #[link_name = "millis"]
    fn _millis() -> u32;
    #[link_name = "delay"]
    fn _delay(ms: u32);
    #[link_name = "print"]
    fn _print(utf8: *const u8, len: usize);
}

pub fn millis() -> u32 {
    unsafe { _millis() }
}

pub fn delay(ms: u32) {
    unsafe {
        _delay(ms);
    }
}

pub fn print(string: &str) {
    unsafe { _print(string.as_ptr(), string.len()) }
}

pub fn println(string: &str) {
    print(string);
    print("\n");
}

pub fn print_num(num: u32) {
    let mut buffer = [0u8; 20];
    let num_buffer = num.numtoa(10, &mut buffer);
    unsafe { _print(num_buffer.as_ptr(), num_buffer.len()) };
}

pub fn println_num(num: u32) {
    print_num(num);
    print("\n");
}

#[panic_handler]
fn handle_panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}
