const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');

let engineProcess = null;
let mainWindow = null;

// ─── Motor Yolu ───
function getEnginePath() {
    if (app.isPackaged) {
        // electron-packager: resources/FlameBot.exe
        return path.join(process.resourcesPath, 'FlameBot.exe');
    } else {
        // Dev modda: gui/../build/FlameBot.exe
        return path.join(__dirname, '..', 'build', 'FlameBot.exe');
    }
}

// ─── Motor Başlatma ───
function startEngine() {
    if (engineProcess) return;

    const enginePath = getEnginePath();
    console.log('[Engine] Starting:', enginePath);

    try {
        engineProcess = spawn(enginePath, [], {
            stdio: ['pipe', 'pipe', 'pipe'],
            windowsHide: true,
            env: process.env
        });

        // stdout'u satır satır oku
        let stdoutBuffer = '';
        engineProcess.stdout.on('data', (data) => {
            stdoutBuffer += data.toString();
            const lines = stdoutBuffer.split('\n');
            // Son eleman eksik satır olabilir, buffer'da tut
            stdoutBuffer = lines.pop();

            for (const line of lines) {
                const trimmed = line.replace(/\r$/, '').trim();
                if (trimmed) {
                    console.log('[Engine →]', trimmed);
                    if (mainWindow && !mainWindow.isDestroyed()) {
                        mainWindow.webContents.send('engine-message', trimmed);
                    }
                }
            }
        });

        engineProcess.stderr.on('data', (data) => {
            console.error('[Engine ERR]', data.toString());
        });

        engineProcess.on('close', (code) => {
            console.log('[Engine] Process exited with code:', code);
            engineProcess = null;
            if (mainWindow && !mainWindow.isDestroyed()) {
                mainWindow.webContents.send('engine-status', 'disconnected');
            }
        });

        engineProcess.on('error', (err) => {
            console.error('[Engine] Spawn error:', err.message);
            engineProcess = null;
            if (mainWindow && !mainWindow.isDestroyed()) {
                mainWindow.webContents.send('engine-status', 'error');
            }
        });

        // Motor başlandığını bildir
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('engine-status', 'connected');
        }

    } catch (err) {
        console.error('[Engine] Failed to start:', err.message);
        engineProcess = null;
    }
}

// ─── Motor Durdurma ───
function stopEngine() {
    if (!engineProcess) return;
    try {
        engineProcess.stdin.write('quit\n');
        setTimeout(() => {
            if (engineProcess) {
                engineProcess.kill();
                engineProcess = null;
            }
        }, 1000);
    } catch (e) {
        if (engineProcess) {
            engineProcess.kill();
            engineProcess = null;
        }
    }
}

// ─── Motora Komut Gönder ───
function sendToEngine(command) {
    if (!engineProcess) return false;
    try {
        console.log('[→ Engine]', command);
        engineProcess.stdin.write(command + '\n');
        return true;
    } catch (e) {
        console.error('[Engine] Write error:', e.message);
        return false;
    }
}

// ─── IPC Handlers ───
ipcMain.handle('engine-start', () => {
    startEngine();
    return !!engineProcess;
});

ipcMain.handle('engine-stop', () => {
    stopEngine();
    return true;
});

ipcMain.on('engine-command', (_event, command) => {
    sendToEngine(command);
});

ipcMain.handle('engine-is-running', () => {
    return !!engineProcess;
});

// ─── Pencere Oluşturma ───
function createWindow() {
    mainWindow = new BrowserWindow({
        width: 960,
        height: 720,
        minWidth: 800,
        minHeight: 600,
        backgroundColor: '#0a0a0f',
        titleBarStyle: 'hidden',
        titleBarOverlay: {
            color: '#0a0a0f',
            symbolColor: '#8a8a9a',
            height: 36
        },
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            nodeIntegration: false
        }
    });

    mainWindow.loadFile('index.html');
    mainWindow.setMenuBarVisibility(false);

    // Sayfa yüklendiğinde motoru otomatik başlat
    mainWindow.webContents.on('did-finish-load', () => {
        startEngine();
    });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
    stopEngine();
    app.quit();
});

app.on('before-quit', () => {
    stopEngine();
});

app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});
