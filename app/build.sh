# Compile
cargo build --release
cp ./target/wasm32-unknown-unknown/release/m5atom_rust_wasm.wasm ./app.wasm

# Optimize (optional)
wasm-opt -O3 app.wasm -o app.wasm
wasm-strip app.wasm

# Convert to WAT
#wasm2wat --generate-names app.wasm -o app.wat

# Convert to C header
xxd -i app.wasm > app.wasm.h
