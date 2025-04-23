function getWasmImports() {
    const env = {
        memory: Module.wasmMemory,
        table:  Module.wasmTable,
        __indirect_function_table: Module.wasmTable,
        __memory_base: Module.__memory_base ?? 1024,
        __table_base:  Module.__table_base  ?? 0,
        __stack_pointer: new WebAssembly.Global({ value: 'i32', mutable: true }, 5242880),
    };

    // Add ALL function imports from the main module
    for (const key in Module) {
        if (typeof Module[key] === 'function' && key.startsWith('_')) {
            const importName = key.substring(1);
            if (!Object.hasOwn(env, importName)) {
                env[importName] = Module[key];
            }
        }
    }

    return {
        env,
        wasi_snapshot_preview1: {},
    };
}

let currentUpdateAndRender = null;

function update_and_render_funWrapper(contextPtr) {
    if (typeof currentUpdateAndRender === 'function') {
        try {
            currentUpdateAndRender(contextPtr);
        } catch (e) {
            console.error("Error executing hot-reloaded update_and_render function:", e);
        }
    }
}

async function reloadWasm() {
    try {
        const wasmUrl = Module.locateFile('game.wasm') + `?t=${Date.now()}`;
        const response = await fetch(wasmUrl);

        if (!response.ok) {
            throw new Error(`Failed to fetch WASM (${response.status}): ${response.statusText}`);
        }

        const imports = getWasmImports();
        const results = await WebAssembly.instantiateStreaming(response, imports);
        const instance = results.instance;

        if (!instance.exports.update_and_render || typeof instance.exports.update_and_render !== 'function') {
            console.error("WASM Load Error: The loaded module does not export 'update_and_render'.", instance.exports);
            throw new Error("Loaded WASM module missing 'update_and_render' export.");
        }

        currentUpdateAndRender = instance.exports.update_and_render;
        console.log("Hot reload successful!");

    } catch (err) {
        console.error('Failed to reload WASM module:', err);
    }
}

Module.onRuntimeInitialized = () => {
    try {
        const update_and_render_funPtr = Module.addFunction(update_and_render_funWrapper, 'vi');

        if (Module._set_update_and_render_func) {
            Module._set_update_and_render_func(update_and_render_funPtr);
            console.log("Callback function set in C module.");
        } else {
            console.error("Error: Main module C function '_set_update_and_render_func' not found/exported.");
            return;
        }

        reloadWasm()
            .then(() => {
                if (currentUpdateAndRender) {
                    setupWebSocket();
                } else {
                    console.error("Initial WASM load failed. WebSocket not started.");
                }
            })
            .catch(err => {
                console.error("Unhandled error during initial WASM load promise:", err);
            });

    } catch (e) {
        console.error("Error during main module runtime initialization:", e);
        if (typeof Module.onAbort === 'function') {
            Module.onAbort(e);
        } else if (typeof Abort === 'function') {
            Abort(e);
        } else {
            console.error("Abort function not available", e);
        }
    }
};

function setupWebSocket() {
    const ws = new WebSocket(`ws://${window.location.hostname}:8081`);
    ws.onmessage = (event) => {
        if (event.data === 'reload') {
            reloadWasm();
        }
    };
    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
    };
    ws.onopen = () => {
        console.log("WebSocket connected and ready for hot reloading");
    };
}

Module['onAbort'] = (reason) => {
    console.error("WASM Aborted:", reason);
    currentUpdateAndRender = null;
};

if (typeof Module.printErr === 'function') {
    const oldErr = Module.printErr;
    Module.printErr = (text) => {
        console.error("Module.printErr:", text);
        oldErr(text);
    };
}