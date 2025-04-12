const status = document.getElementById('status');

function getWasmImports() {
    return {
        env: {
            memory: Module.wasmMemory,
            table: Module.wasmTable,
            __memory_base: new WebAssembly.Global({value: 'i32', mutable: false}, 1024),
            __table_base: new WebAssembly.Global({value: 'i32', mutable: false}, 0),
            SDL_SetRenderDrawColor: Module._SDL_SetRenderDrawColor,

            SDL_RenderClear: Module._SDL_RenderClear,
            SDL_RenderPresent: Module._SDL_RenderPresent,
            SDL_RenderFillRect: Module._SDL_RenderFillRect,
        },
        wasi_snapshot_preview1: {}
    };
}

function updateFuncWrapper(contextPtr) {
    if (typeof currentUpdate === 'function') {
        return currentUpdate(contextPtr);
    }
    console.warn('No update function available!');
    return 0;
}

let currentUpdate;

async function reloadWasm() {
    status.textContent = 'Loading logic...';
    try {
        const result = await WebAssembly.instantiateStreaming(
            fetch(`/hot_reload/build/game.wasm?t=${Date.now()}`),
            getWasmImports()
        );

        currentUpdate = result.instance.exports.update;
        console.log('WASM module loaded, update function:', currentUpdate);

        status.textContent = 'Hot Reloaded!';
    } catch (err) {
        console.error('Failed to reload WASM module:', err);
        status.textContent = 'Reload failed!';
    }
}

Module.onRuntimeInitialized = () => {
    const updateFuncPtr = Module.addFunction(updateFuncWrapper, 'vi');
    Module._setUpdateFunc(updateFuncPtr);

    reloadWasm();

    const ws = new WebSocket('ws://localhost:8081');
    ws.onopen = () => (status.textContent = 'Connected. Edit game.c to reload.');
    ws.onmessage = () => reloadWasm();
    ws.onerror = () => (status.textContent = 'WebSocket error');
};