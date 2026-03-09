// app.js — Uygulama (Motor Bağlantılı)

document.addEventListener('DOMContentLoaded', () => {
    const game = new GameState();
    const boardContainer = document.getElementById('board-container');
    const board = new ChessBoard(game, boardContainer);

    // ─── UI Elemanları ───
    const turnDot = document.getElementById('turn-dot');
    const turnText = document.getElementById('turn-text');
    const statusText = document.getElementById('status-text');
    const moveList = document.getElementById('move-list');
    const moveCount = document.getElementById('move-count');
    const modeBadge = document.getElementById('mode-badge');
    const engineStatusEl = document.getElementById('engine-status');

    const btnNewGame = document.getElementById('btn-new-game');
    const btnUndo = document.getElementById('btn-undo');
    const btnFlip = document.getElementById('btn-flip');

    // ═════════════════════════════════════════════
    //  MOTOR BAĞLANTISI
    // ═════════════════════════════════════════════
    let engineReady = false;
    let engineThinking = false;
    let uciInitDone = false;

    function updateEngineStatusUI(status) {
        if (status === 'thinking') {
            engineStatusEl.innerHTML = '<div class="dot-online"></div><span>Düşünüyor...</span>';
        } else if (status === 'connected' || status === 'ready') {
            engineStatusEl.innerHTML = '<div class="dot-online"></div><span>Bağlı</span>';
        } else {
            engineStatusEl.innerHTML = '<div class="dot-offline"></div><span>Bağlı değil</span>';
        }
    }

    // Motor mesajlarını dinle
    if (window.electronAPI) {
        window.electronAPI.onEngineMessage((msg) => {
            handleEngineMessage(msg);
        });

        window.electronAPI.onEngineStatus((status) => {
            if (status === 'connected') {
                // UCI handshake başlat
                sendEngineCommand('uci');
            } else if (status === 'disconnected' || status === 'error') {
                engineReady = false;
                uciInitDone = false;
                updateEngineStatusUI('disconnected');
            }
        });
    }

    function sendEngineCommand(cmd) {
        if (window.electronAPI) {
            window.electronAPI.sendToEngine(cmd);
        }
    }

    function handleEngineMessage(msg) {
        // UCI handshake
        if (msg === 'uciok') {
            sendEngineCommand('isready');
            return;
        }

        if (msg === 'readyok') {
            engineReady = true;
            uciInitDone = true;
            updateEngineStatusUI('ready');

            // Eğer bot sırası bekliyorduysa şimdi hamle yap
            if (shouldBotPlayNow()) {
                requestEngineMove();
            }
            return;
        }

        // bestmove yanıtı
        if (msg.startsWith('bestmove')) {
            const parts = msg.split(/\s+/);
            const bestmove = parts[1];
            if (bestmove && bestmove !== '(none)' && bestmove !== '0000') {
                applyEngineMove(bestmove);
            }
            engineThinking = false;
            updateEngineStatusUI('ready');
            return;
        }

        // info satırları (ileride eval bar vs. için kullanılabilir)
        // şimdilik yoksay
    }

    function applyEngineMove(uciMove) {
        // UCI formatı: "e2e4" veya "e7e8q"
        if (uciMove.length < 4) return;

        const fromCol = uciMove.charCodeAt(0) - 97; // 'a' = 0
        const fromRow = 8 - parseInt(uciMove[1]);     // '1' = 7, '8' = 0
        const toCol = uciMove.charCodeAt(2) - 97;
        const toRow = 8 - parseInt(uciMove[3]);

        // Piyon terfisi kontrolü
        // game.js otomatik vezir terfi yapıyor, motor da 'q' gönderiyor — uyumlu

        if (game.makeMove(fromRow, fromCol, toRow, toCol)) {
            board.clearSelection();
            board.render();
            updateUI();

            // BvB modunda sıradaki bot hamlesi
            if (currentMode === 'bvb' && !game.gameOver) {
                setTimeout(() => requestEngineMove(), 400);
            }
        }
    }

    function requestEngineMove() {
        if (!engineReady || engineThinking || game.gameOver) return;

        engineThinking = true;
        updateEngineStatusUI('thinking');

        // Pozisyonu gönder
        const fen = game.toFEN();
        sendEngineCommand('position fen ' + fen);

        // Arama başlat
        sendEngineCommand('go depth 8');
    }

    function shouldBotPlayNow() {
        if (game.gameOver) return false;
        if (currentMode === 'pvb' && game.turn === 'b') return true;
        if (currentMode === 'bvb') return true;
        return false;
    }

    // ═════════════════════════════════════════════
    //  SEKME SİSTEMİ
    // ═════════════════════════════════════════════
    const tabBtns = document.querySelectorAll('.tab-btn');
    tabBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            const tab = btn.dataset.tab;
            tabBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            document.getElementById('tab-content-' + tab).classList.add('active');
            if (tab === 'settings') refreshHistoryList();
        });
    });

    // ═════════════════════════════════════════════
    //  TEMA SİSTEMİ
    // ═════════════════════════════════════════════
    const themeChips = document.querySelectorAll('.theme-chip');
    const savedTheme = localStorage.getItem('chess-theme') || 'obsidian';
    setTheme(savedTheme);

    themeChips.forEach(chip => {
        chip.addEventListener('click', () => setTheme(chip.dataset.theme));
    });

    function setTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem('chess-theme', theme);
        themeChips.forEach(c => c.classList.toggle('active', c.dataset.theme === theme));
    }

    // ═════════════════════════════════════════════
    //  OYUN MODU SİSTEMİ
    // ═════════════════════════════════════════════
    let currentMode = 'pvp';
    let bvbInterval = null;

    const modeLabels = { pvp: '👤👤 İnsan vs İnsan', pvb: '👤🤖 İnsan vs Bot', bvb: '🤖🤖 Bot vs Bot' };

    const modeBtns = document.querySelectorAll('.mode-btn');
    modeBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            const newMode = btn.dataset.mode;
            if (newMode === currentMode) return;

            stopBvb();
            engineThinking = false;
            currentMode = newMode;
            modeBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            modeBadge.textContent = modeLabels[currentMode];

            // Yeni oyun başlat
            sendEngineCommand('ucinewgame');
            savedCurrentGame = false;
            game.reset();
            board.clearSelection();
            board.render();
            updateUI();

            // Bot modlarında ilk hamleyi başlat
            if (currentMode === 'bvb' && engineReady) {
                setTimeout(() => requestEngineMove(), 500);
            }
        });
    });

    modeBadge.textContent = modeLabels[currentMode];

    function stopBvb() {
        if (bvbInterval) { clearInterval(bvbInterval); bvbInterval = null; }
    }

    // ═════════════════════════════════════════════
    //  RASTGELE BOT (Motor yoksa fallback)
    // ═════════════════════════════════════════════
    function makeFallbackBotMove() {
        if (game.gameOver) return;
        const all = [];
        for (let r = 0; r < 8; r++)
            for (let c = 0; c < 8; c++) {
                const p = game.getPiece(r, c);
                if (p && game.isOwnPiece(p))
                    for (const m of game.getValidMoves(r, c))
                        all.push({ fromRow: r, fromCol: c, toRow: m.row, toCol: m.col });
            }
        if (all.length === 0) return;
        const captures = all.filter(m => game.getPiece(m.toRow, m.toCol));
        const pool = captures.length > 0 ? captures : all;
        const chosen = pool[Math.floor(Math.random() * pool.length)];
        game.makeMove(chosen.fromRow, chosen.fromCol, chosen.toRow, chosen.toCol);
        board.clearSelection(); board.render(); updateUI();
    }

    function triggerBotMove() {
        if (game.gameOver) return;

        if (engineReady) {
            requestEngineMove();
        } else {
            // Motor yoksa fallback
            setTimeout(() => makeFallbackBotMove(), 300);
        }
    }

    // ═════════════════════════════════════════════
    //  UI GÜNCELLEME
    // ═════════════════════════════════════════════
    function updateUI() {
        turnDot.className = 'turn-dot ' + (game.turn === 'w' ? 'white' : 'black');
        turnText.textContent = game.turn === 'w' ? 'Beyaz' : 'Siyah';

        if (game.gameOver) {
            if (game.gameResult === 'checkmate') {
                const winner = game.turn === 'w' ? 'Siyah' : 'Beyaz';
                statusText.textContent = `Şah Mat! ${winner} kazandı`;
                statusText.className = 'status-badge checkmate';
            } else {
                statusText.textContent = 'Pat — Berabere';
                statusText.className = 'status-badge stalemate';
            }
            stopBvb();
            saveGameToHistory();
        } else if (game.isInCheck()) {
            statusText.textContent = 'Şah!';
            statusText.className = 'status-badge in-check';
        } else {
            statusText.textContent = 'Devam ediyor';
            statusText.className = 'status-badge';
        }

        moveCount.textContent = `Hamle: ${game.moveHistory.length}`;
        updateMoveList();
    }

    function updateMoveList() {
        const history = game.moveHistory;
        if (history.length === 0) {
            moveList.innerHTML = '<div class="move-list-empty">İlk hamleyi yapın</div>';
            return;
        }
        moveList.innerHTML = '';
        for (let i = 0; i < history.length; i += 2) {
            const moveNum = Math.floor(i / 2) + 1;
            const row = document.createElement('div');
            row.className = 'move-row';
            const numEl = document.createElement('span');
            numEl.className = 'move-number';
            numEl.textContent = moveNum + '.';
            const whiteMove = document.createElement('span');
            whiteMove.className = 'move-text';
            whiteMove.textContent = formatMove(history[i]);
            row.appendChild(numEl);
            row.appendChild(whiteMove);
            if (i + 1 < history.length) {
                const blackMove = document.createElement('span');
                blackMove.className = 'move-text';
                blackMove.textContent = formatMove(history[i + 1]);
                row.appendChild(blackMove);
            }
            moveList.appendChild(row);
        }
        moveList.scrollTop = moveList.scrollHeight;
    }

    function formatMove(move) {
        const files = 'abcdefgh';
        const ranks = '87654321';
        let notation = '';
        const piece = move.promotedFrom || move.piece;
        const type = piece.toLowerCase();
        if (move.castlingType === 'K' || move.castlingType === 'k') return 'O-O';
        if (move.castlingType === 'Q' || move.castlingType === 'q') return 'O-O-O';
        const sym = { k: '♚', q: '♛', r: '♜', b: '♝', n: '♞' };
        if (type !== 'p') notation += sym[type];
        if (move.captured || move.enPassantCapture) {
            if (type === 'p') notation += files[move.from.col];
            notation += '×';
        }
        notation += files[move.to.col] + ranks[move.to.row];
        if (move.promotedFrom) notation += '=♛';
        return notation;
    }

    // ═════════════════════════════════════════════
    //  OYUN GEÇMİŞİ
    // ═════════════════════════════════════════════
    const HISTORY_KEY = 'chess-game-history';
    const MAX_HISTORY = 50;
    let savedCurrentGame = false;

    function getHistory() {
        try { return JSON.parse(localStorage.getItem(HISTORY_KEY) || '[]'); }
        catch { return []; }
    }
    function saveHistory(history) {
        localStorage.setItem(HISTORY_KEY, JSON.stringify(history.slice(0, MAX_HISTORY)));
    }

    function saveGameToHistory() {
        if (savedCurrentGame) return;
        if (game.moveHistory.length < 2) return;
        savedCurrentGame = true;
        const history = getHistory();
        const record = {
            id: Date.now(),
            date: new Date().toLocaleString('tr-TR', { day: '2-digit', month: '2-digit', year: 'numeric', hour: '2-digit', minute: '2-digit' }),
            mode: currentMode, modeLabel: modeLabels[currentMode],
            result: game.gameResult,
            winner: game.gameResult === 'checkmate' ? (game.turn === 'w' ? 'black' : 'white') : 'draw',
            totalMoves: game.moveHistory.length,
            moves: game.moveHistory.map(m => ({
                from: m.from, to: m.to, piece: m.piece, captured: m.captured,
                castlingType: m.castlingType || null, enPassantCapture: m.enPassantCapture || null,
                promotedFrom: m.promotedFrom || null, castlingRights: m.castlingRights,
                enPassantTarget: m.enPassantTarget, halfMoveClock: m.halfMoveClock
            }))
        };
        history.unshift(record);
        saveHistory(history);
    }

    function refreshHistoryList() {
        const list = document.getElementById('history-list');
        const history = getHistory();
        if (history.length === 0) {
            list.innerHTML = '<div class="history-empty">Henüz kayıtlı oyun yok</div>';
            return;
        }
        list.innerHTML = '';
        history.forEach((record, idx) => {
            const item = document.createElement('div');
            item.className = 'history-item';
            const resultIcon = record.result === 'checkmate'
                ? (record.winner === 'white' ? '⬜' : '⬛') : '🤝';
            item.innerHTML = `
                <span class="hi-result">${resultIcon}</span>
                <div class="hi-info">
                    <div class="hi-mode">${record.modeLabel}</div>
                    <div class="hi-meta">${record.date} · ${record.totalMoves} hamle</div>
                </div>
                <button class="hi-play" data-idx="${idx}">▶ Oynat</button>`;
            item.querySelector('.hi-play').addEventListener('click', (e) => {
                e.stopPropagation();
                startReplay(record);
            });
            list.appendChild(item);
        });
    }

    document.getElementById('btn-clear-history').addEventListener('click', () => {
        localStorage.removeItem(HISTORY_KEY);
        refreshHistoryList();
    });

    // ═════════════════════════════════════════════
    //  YENİDEN OYNATMA
    // ═════════════════════════════════════════════
    let replayData = null;
    let replayIndex = 0;
    let replayPlaying = false;
    let replayTimer = null;
    let replayGame = null;

    const replayBar = document.getElementById('replay-bar');
    const replayTitle = document.getElementById('replay-title');
    const replayProgressBar = document.getElementById('replay-progress-bar');

    function startReplay(record) {
        tabBtns.forEach(b => b.classList.remove('active'));
        document.getElementById('tab-game').classList.add('active');
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        document.getElementById('tab-content-game').classList.add('active');
        stopBvb(); engineThinking = false;

        replayData = record; replayIndex = 0;
        replayGame = new GameState();
        board.game = replayGame;
        board.clearSelection(); board.render();
        replayBar.classList.remove('hidden');
        replayTitle.textContent = `${record.modeLabel} · ${record.date}`;
        updateReplayUI();
    }

    function stopReplay() {
        if (replayTimer) { clearInterval(replayTimer); replayTimer = null; }
        replayPlaying = false; replayData = null; replayGame = null;
        replayBar.classList.add('hidden');
        board.game = game;
        board.clearSelection(); board.render(); updateUI();
    }

    function replayGoTo(idx) {
        if (!replayData) return;
        replayGame = new GameState();
        const moves = replayData.moves;
        for (let i = 0; i < idx && i < moves.length; i++)
            replayGame.makeMove(moves[i].from.row, moves[i].from.col, moves[i].to.row, moves[i].to.col);
        replayIndex = idx;
        board.game = replayGame;
        board.clearSelection(); board.render();
        updateReplayUI();
    }

    function updateReplayUI() {
        if (!replayData) return;
        const total = replayData.moves.length;
        replayProgressBar.style.width = (total > 0 ? (replayIndex / total) * 100 : 0) + '%';

        turnDot.className = 'turn-dot ' + (replayGame.turn === 'w' ? 'white' : 'black');
        turnText.textContent = replayGame.turn === 'w' ? 'Beyaz' : 'Siyah';
        moveCount.textContent = `Hamle: ${replayIndex} / ${total}`;

        if (replayIndex >= total) {
            if (replayData.result === 'checkmate') {
                const w = replayData.winner === 'white' ? 'Beyaz' : 'Siyah';
                statusText.textContent = `Şah Mat! ${w} kazandı`;
                statusText.className = 'status-badge checkmate';
            } else {
                statusText.textContent = 'Pat — Berabere';
                statusText.className = 'status-badge stalemate';
            }
        } else {
            statusText.textContent = 'Yeniden Oynatma';
            statusText.className = 'status-badge';
        }

        // Replay hamle listesi
        const moves = replayData.moves;
        if (replayIndex === 0) {
            moveList.innerHTML = '<div class="move-list-empty">▶ Oynatmaya başlayın</div>';
        } else {
            moveList.innerHTML = '';
            for (let i = 0; i < replayIndex; i += 2) {
                const row = document.createElement('div');
                row.className = 'move-row';
                row.innerHTML = `<span class="move-number">${Math.floor(i / 2) + 1}.</span><span class="move-text">${formatMove(moves[i])}</span>`;
                if (i + 1 < replayIndex) row.innerHTML += `<span class="move-text">${formatMove(moves[i + 1])}</span>`;
                moveList.appendChild(row);
            }
            moveList.scrollTop = moveList.scrollHeight;
        }
    }

    document.getElementById('replay-start').addEventListener('click', () => replayGoTo(0));
    document.getElementById('replay-end').addEventListener('click', () => { if (replayData) replayGoTo(replayData.moves.length); });
    document.getElementById('replay-prev').addEventListener('click', () => { if (replayIndex > 0) replayGoTo(replayIndex - 1); });
    document.getElementById('replay-next').addEventListener('click', () => { if (replayData && replayIndex < replayData.moves.length) replayGoTo(replayIndex + 1); });
    document.getElementById('replay-exit').addEventListener('click', stopReplay);

    document.getElementById('replay-play').addEventListener('click', () => {
        if (!replayData) return;
        if (replayPlaying) {
            clearInterval(replayTimer); replayTimer = null;
            replayPlaying = false;
            document.getElementById('replay-play').textContent = '▶';
        } else {
            if (replayIndex >= replayData.moves.length) replayGoTo(0);
            replayPlaying = true;
            document.getElementById('replay-play').textContent = '⏸';
            replayTimer = setInterval(() => {
                if (replayIndex >= replayData.moves.length) {
                    clearInterval(replayTimer); replayTimer = null;
                    replayPlaying = false;
                    document.getElementById('replay-play').textContent = '▶';
                    return;
                }
                replayGoTo(replayIndex + 1);
            }, 800);
        }
    });

    // ═════════════════════════════════════════════
    //  HAMLE CALLBACK & BUTONLAR
    // ═════════════════════════════════════════════
    board.onMoveCallback = (moveStr) => {
        updateUI();

        // Bot sırası
        if ((currentMode === 'pvb' && game.turn === 'b') && !game.gameOver) {
            triggerBotMove();
        }
    };

    btnNewGame.addEventListener('click', () => {
        if (replayData) { stopReplay(); return; }
        stopBvb(); engineThinking = false; savedCurrentGame = false;
        sendEngineCommand('ucinewgame');
        game.reset();
        board.clearSelection(); board.render(); updateUI();

        if (currentMode === 'bvb' && engineReady) {
            setTimeout(() => requestEngineMove(), 500);
        }
    });

    btnUndo.addEventListener('click', () => {
        if (replayData) return;
        if (currentMode === 'bvb') return;
        if (engineThinking) return;
        if (currentMode === 'pvb') { game.undoMove(); game.undoMove(); }
        else { game.undoMove(); }
        board.clearSelection(); board.render(); updateUI();
    });

    btnFlip.addEventListener('click', () => board.flipBoard());

    // ─── İlk Render ───
    board.render();
    updateUI();
});
