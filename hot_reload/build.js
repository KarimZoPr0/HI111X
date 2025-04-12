const { exec } = require('child_process');
const ws = require('ws');
const chokidar = require('chokidar');

const wss = new ws.Server({ port: 8081 });
const notify = () => {
    wss.clients.forEach(client => {
        if (client.readyState === ws.OPEN) {
            client.send('reload', err => {
                if (err) console.error('Error sending reload:', err);
            });
        }
    });
};

function runCommand(command) {
    return new Promise((resolve, reject) => {
        exec(command, (err, stdout, stderr) => {
            if (err) {
                reject(stderr || err);
            } else {
                resolve(stdout);
            }
        });
    });
}

function getMainCmd() {
    return [
        'emcc backends/sdl2/sdl_app.c',
        '-o build/sdl_app.js',
        '-sWASM=1',
        '-sUSE_SDL=2',
        '-sMAIN_MODULE=2',
        '-sEXPORTED_FUNCTIONS="[_main,_setUpdateFunc,_SDL_SetRenderDrawColor,_SDL_RenderClear,_SDL_RenderFillRect,_SDL_RenderPresent]"',
        '-sEXPORTED_RUNTIME_METHODS="[cwrap,addFunction,wasmMemory,wasmTable]"',
        '-sALLOW_TABLE_GROWTH'
    ].join(' ');
}

function getLogicCmd() {
    return [
        'emcc game/game.c',
        '-O2',
        '-sSIDE_MODULE=2',
        '-sUSE_SDL=2',
        '-sIMPORTED_MEMORY',
        '-sWASM_BIGINT',
        '-s"EXPORTED_FUNCTIONS=[_update]"',
        '-o build/game.wasm'
    ].join(' ');
}

async function buildMain() {
    try {
        const mainOutput = await runCommand(getMainCmd());
        console.log('Main module built successfully:');
        if (mainOutput.trim()) {
            process.stdout.write(mainOutput.trimEnd() + '\n');
        }
    } catch (error) {
        console.error('Build error for main module:', error);
    }
}

async function buildLogic() {
    try {
        const logicOutput = await runCommand(getLogicCmd());
        console.log('Logic module built successfully:');
        if (logicOutput.trim()) {
            process.stdout.write(logicOutput.trimEnd() + '\n');
        }
        notify();
    } catch (error) {
        console.error('Logic module build error:', error);
    }
}


buildMain();
buildLogic();

chokidar.watch('game/game.c').on('change', buildLogic);
