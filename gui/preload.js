const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
    // Motor komutları
    sendToEngine: (command) => ipcRenderer.send('engine-command', command),
    onEngineMessage: (callback) => ipcRenderer.on('engine-message', (_event, msg) => callback(msg)),
    onEngineStatus: (callback) => ipcRenderer.on('engine-status', (_event, status) => callback(status)),

    // Motor yönetimi
    startEngine: () => ipcRenderer.invoke('engine-start'),
    stopEngine: () => ipcRenderer.invoke('engine-stop'),
    isEngineRunning: () => ipcRenderer.invoke('engine-is-running'),

    // Platform bilgisi
    platform: process.platform
});
