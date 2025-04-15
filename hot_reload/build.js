const { exec } = require('child_process');
const { performance } = require('perf_hooks');
const ws = require('ws');
const chokidar = require('chokidar');

const wss = new ws.Server({ port: 8081 });

const notify = () => {
    console.log('Notifying clients to reload...');
    wss.clients.forEach(client => {
        if (client.readyState === ws.OPEN) {
            client.send('reload', err => {
                if (err) console.error('Error sending reload notification:', err);
            });
        }
    });
};

function runCommand(command) {
    return new Promise((resolve, reject) => {
        console.log(`Executing: ${command}`);
        exec(command, (err, stdout, stderr) => {
            if (err) {
                console.error(`Error: ${stderr || err.message}`);
                resolve({ error: stderr || err, stdout });
            } else {
                if (stdout) console.log(`Output: ${stdout}`);
                if (stderr) console.warn(`Warning: ${stderr}`);
                resolve({ error: null, stdout });
            }
        });
    });
}

function getMainCmd() {
    return [
        'emcc sdl_app.c', '-o build/sdl_app.js',
        '-sUSE_SDL=2',
        '-sMAIN_MODULE=2',
        '-sEXPORTED_FUNCTIONS="[_main,_set_update_and_render_func,_SDL_SetRenderDrawColor,_SDL_RenderClear,_SDL_RenderFillRect,_SDL_RenderPresent]"',
        '-sEXPORTED_RUNTIME_METHODS="[cwrap,addFunction,wasmMemory,wasmTable,printErr,abort,asm]"', // Added 'asm'
        '-sALLOW_MEMORY_GROWTH=1',
        '-sASSERTIONS=1',
    ].join(' ');
}

function getGameCmd() {
    return [
        'emcc game/game.c', '-o build/game.wasm',
        '-O2',
        '-sSIDE_MODULE=2',
        '-sEXPORTED_FUNCTIONS="[_update_and_render]"',
    ].join(' ');
}

async function buildMain() {
    console.log('\nBuilding main module...');
    const start = performance.now();
    try {
        const { error } = await runCommand(getMainCmd());
        if (error) {
            console.error('Main module build failed.');
        } else {
            console.log('Main module build successful.');
        }
    } catch (error) {
        console.error('Unexpected error during main build process:', error);
    } finally {
        const duration = (performance.now() - start).toFixed(2);
        console.log(`Main module build process finished in ${duration}ms`);
    }
}

async function buildLogic() {
    console.log('\nBuilding logic side module...');
    const start = performance.now();
    let success = false;
    try {
        const { error } = await runCommand(getGameCmd());
        if (error) {
            console.error('Logic module build failed.');
        } else {
            console.log('Logic module build successful.');
            success = true;
        }
    } catch (error) {
        console.error('Unexpected error during logic build process:', error);
    } finally {
        const duration = (performance.now() - start).toFixed(2);
        console.log(`Logic module build process finished in ${duration}ms`);
        if (success) {
            notify();
        }
    }
}

(async () => {
    await buildMain();
    await buildLogic();

    console.log('\nWatching game/game.c for changes...');
    chokidar.watch('game/game.c').on('change', (path) => {
        console.log(`\nDetected change in ${path}`);
        buildLogic();
    });

    wss.on('connection', ws => {
        console.log('Client connected.');
        ws.on('close', () => console.log('Client disconnected.'));
        ws.on('error', (error) => console.error('WebSocket server error:', error));
    });

    console.log('WebSocket server started on port 8081.');
})();