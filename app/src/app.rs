#![no_std]

mod arduino;
use arduino::*;

enum PreviousTick {
    Tick,
    Tock,
}

struct App {
    previous_tick: PreviousTick,
}

impl App {
    fn new() -> Self {
        App {
            previous_tick: PreviousTick::Tock,
        }
    }

    fn tick(&mut self) {
        println_num(millis());
        match self.previous_tick {
            PreviousTick::Tick => {
                println("Tock!");
                self.previous_tick = PreviousTick::Tock;
            }
            PreviousTick::Tock => {
                println("Tick!");
                self.previous_tick = PreviousTick::Tick;
            }
        };
    }
}

#[no_mangle]
pub extern "C" fn _start() {
    println("Rust is running 😎");

    let mut app = App::new();
    loop {
        app.tick();
        delay(1000);
    }
}
