const {spawn} = require('child_process');
const {performance} = require('perf_hooks');
const chokidar = require('chokidar');
const WebSocket = require('ws');

const wss = new WebSocket.Server({port:8081});
const notify = () => wss.clients.forEach(c => c.readyState === 1 && c.send('reload'));

function run(name, cmd, args, notifyOnSuccess=false)
{
    const t0 = performance.now();
    spawn(cmd, args, {stdio:'inherit', shell:true})
        .on('exit', code=>{
            const dt = (performance.now()-t0)|0;
            if(code===0){ console.log(`${name} ✓ (${dt} ms)`); if(notifyOnSuccess) notify(); }
            else console.error(`${name} ✗ (${code})`);
        });
}

const MAIN = [
    'sdl_app.c','-o','build/sdl_app.js',
    '-sUSE_SDL=2','-sUSE_SDL_NET=2','-lwebsocket.js',
    '-sWEBSOCKET_URL="ws://localhost:12345"',
    '-sMAIN_MODULE=1','-sEXPORT_ALL=1',
    '-sALLOW_MEMORY_GROWTH=1',
    '-sEXPORTED_RUNTIME_METHODS="[cwrap,addFunction,wasmMemory,wasmTable]"',
    '-sASSERTIONS=1',
    '-O0',
    '-sWASM_BIGINT',
];

const SIDE = [
    'game/game.c','-o','build/game.wasm',
    '-O0',
    '-sSIDE_MODULE=2',
];

run('main', 'emcc', MAIN);
run('side', 'emcc', SIDE, true);

chokidar.watch('game/game.c').on('change',()=>run('logic','emcc',SIDE,true));