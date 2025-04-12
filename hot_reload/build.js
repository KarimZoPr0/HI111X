const { execSync, exec } = require('child_process');
const fs = require('fs');
const ws = require('ws');
const chokidar = require('chokidar');

if (!fs.existsSync('build')) fs.mkdirSync('build');
const wss = new ws.Server({ port: 8081 });
const notify = () => wss.clients.forEach(c => c.send('reload'));

// Step 1: Pre-build with symbol map
execSync(`emcc backends/sdl2/sdl_app.c -o build/sdl_app.js -sWASM=1 -sUSE_SDL=2 -sMAIN_MODULE=2 \
--emit-symbol-map \
-sEXPORTED_FUNCTIONS="['_main']" \
-sEXPORTED_RUNTIME_METHODS="['cwrap','addFunction','wasmMemory','wasmTable']" \
-sALLOW_TABLE_GROWTH`);

console.log('Prebuilt with symbol map');

// Step 2: Load and parse symbol map
const symbolsPath = 'build/sdl_app.js.symbols';
const symbols = fs.readFileSync(symbolsPath, 'utf-8')
    .split('\n')
    .filter(name => name && (name === '_main' || name.startsWith('_SDL_')));

// Step 3: Rebuild with all SDL symbols + main
const exportedFunctions = `-sEXPORTED_FUNCTIONS="[${symbols.map(s => `'${s}'`).join(',')}]"`;

const mainBuildCmd = `emcc backends/sdl2/sdl_app.c -o build/sdl_app.js -sWASM=1 -sUSE_SDL=2 -sMAIN_MODULE=2 \
${exportedFunctions} \
-sEXPORTED_RUNTIME_METHODS="['cwrap','addFunction','wasmMemory','wasmTable']" \
-sALLOW_TABLE_GROWTH`;

exec(mainBuildCmd, (err, stdout, stderr) => {
    if (!err) {
        console.log('Main module rebuilt successfully');
        console.log(stdout);
    } else {
        console.error('Error building main module:');
        console.error(stderr || err);
    }
});

// Step 4: Build side module (logic)
function buildLogic() {
    exec(`emcc game/game.c -O2 -sSIDE_MODULE=2 -sUSE_SDL=2 -sIMPORTED_MEMORY -sWASM_BIGINT \
    -s"EXPORTED_FUNCTIONS=['_update']" -o build/game.wasm`, (e) => {
        if (!e) {
            console.log('Logic built');
            notify();
        } else {
            console.error(e);
        }
    });
}

buildLogic();
chokidar.watch('game/game.c').on('change', buildLogic);
