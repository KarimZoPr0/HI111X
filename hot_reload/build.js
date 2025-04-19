const { spawn } = require('child_process');
const { performance } = require('perf_hooks');
const WebSocket = require('ws');
const chokidar  = require('chokidar');

const wss = new WebSocket.Server({ port: 8081 });

const notify = () => {
    for (const client of wss.clients) {
        if (client.readyState === WebSocket.OPEN) {
            try {
                client.send('reload');
            } catch (err) {
                console.error('Error sending reload:', err);
            }
        }
    }
};

function build(name, cmd, args, { notifyOnSuccess = false } = {}) {
    const start = performance.now();
    console.log(`[${name}] spawning: ${cmd} ${args.join(' ')}`);

    const child = spawn(cmd, args, { stdio: 'inherit', shell: true });
    child.on('error', err => {
        console.error(`[${name}] spawn error:`, err);
    });
    child.on('exit', code => {
        const ms = (performance.now() - start).toFixed(0);
        if (code === 0) {
            console.log(`[${name}] succeeded in ${ms}ms`);
            if (notifyOnSuccess) notify();
        } else {
            console.error(`[${name}] failed (code ${code}) in ${ms}ms`);
        }
    });
}

const MAIN_ARGS = [
    'sdl_app.c',
    '-o', 'build/sdl_app.js',
    '-sUSE_SDL=2',
    '-sUSE_SDL_IMAGE=2',
    '-sSDL2_IMAGE_FORMATS=png',
    '-sMAIN_MODULE=1',
    '-sEXPORT_ALL=1',
    '-sALLOW_MEMORY_GROWTH=1',
    '-sEXPORTED_RUNTIME_METHODS="[cwrap,addFunction,wasmMemory,wasmTable]"',
    '-sASSERTIONS=1',
    '--preload-file', 'gfx',
];

const LOGIC_ARGS = [
    'game/game.c',
    '-o', 'build/game.wasm',
    '-O2',
    '-sSIDE_MODULE=2',
];

build('Main Module',  'emcc', MAIN_ARGS);
build('Logic Module', 'emcc', LOGIC_ARGS, { notifyOnSuccess: true });

chokidar.watch('game/game.c').on('change', path => {
    console.log(`File changed: ${path}, rebuilding logic…`);
    build('Logic Module', 'emcc', LOGIC_ARGS, { notifyOnSuccess: true });
});

wss.on('connection', socket => {
    console.log('Client connected');
    socket.on('close',   () => console.log('Client disconnected'));
    socket.on('error',   e  => console.error('WebSocket error:', e));
});
