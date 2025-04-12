const status = document.getElementById('status');

function buildEnvImports() {
    const env = {
        memory: Module.wasmMemory,
        table: Module.wasmTable,
        __memory_base: new WebAssembly.Global({ value: 'i32', mutable: false }, 1024),
        __table_base: new WebAssembly.Global({ value: 'i32', mutable: false }, 0),
    };

    // Automatically forward all exported functions from Module._<func>
    for (const key in Module) {
        if (key.startsWith('_') && typeof Module[key] === 'function') {
            const wasmFuncName = key.slice(1); // Strip leading underscore
            env[wasmFuncName] = Module[key];
        }
    }

    return env;
}

function getWasmImports() {
    return {
        env: buildEnvImports(),
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
