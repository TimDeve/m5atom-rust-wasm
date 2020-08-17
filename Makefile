.PHONY: build upload monitor clean clean_io clean_rust

build: app/app.wasm.h
	platformio run

upload: app/app.wasm.h
	platformio run -t upload

app/app.wasm.h: $(wildcard app/src/*.rs)
	@(cd app && ./build)

monitor:
	platformio device monitor

clean: clean_io clean_rust

clean_io:
	platformio run -t clean

clean_rust:
	@(cd app && ./clean)

