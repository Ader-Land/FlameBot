// game.js — Oyun durumu ve hamle mantığı

class GameState {
    constructor() {
        this.reset();
    }

    reset() {
        // Başlangıç pozisyonu (8×8 matris, [rank][file] — rank 0 = rank 8, rank 7 = rank 1)
        this.board = this.getInitialBoard();
        this.turn = 'w'; // 'w' veya 'b'
        this.moveHistory = [];
        this.selectedSquare = null;
        this.validMoves = [];
        this.lastMove = null;
        this.gameOver = false;
        this.gameResult = '';

        // Rok hakları
        this.castlingRights = { K: true, Q: true, k: true, q: true };

        // En passant hedefi (null veya {row, col})
        this.enPassantTarget = null;

        // Hamle sayacı
        this.halfMoveClock = 0;
        this.fullMoveNumber = 1;
    }

    getInitialBoard() {
        return [
            ['r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'], // rank 8 (siyah)
            ['p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'], // rank 7
            [null, null, null, null, null, null, null, null], // rank 6
            [null, null, null, null, null, null, null, null], // rank 5
            [null, null, null, null, null, null, null, null], // rank 4
            [null, null, null, null, null, null, null, null], // rank 3
            ['P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'], // rank 2
            ['R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R']  // rank 1 (beyaz)
        ];
    }

    getPiece(row, col) {
        if (row < 0 || row > 7 || col < 0 || col > 7) return null;
        return this.board[row][col];
    }

    isWhitePiece(piece) {
        return piece && piece === piece.toUpperCase();
    }

    isBlackPiece(piece) {
        return piece && piece === piece.toLowerCase();
    }

    isOwnPiece(piece) {
        if (!piece) return false;
        return this.turn === 'w' ? this.isWhitePiece(piece) : this.isBlackPiece(piece);
    }

    isEnemyPiece(piece) {
        if (!piece) return false;
        return this.turn === 'w' ? this.isBlackPiece(piece) : this.isWhitePiece(piece);
    }

    // Basit hamle geçerliliği (tam kural seti)
    getValidMoves(row, col) {
        const piece = this.getPiece(row, col);
        if (!piece || !this.isOwnPiece(piece)) return [];

        const type = piece.toLowerCase();
        let moves = [];

        switch (type) {
            case 'p': moves = this.getPawnMoves(row, col); break;
            case 'r': moves = this.getRookMoves(row, col); break;
            case 'n': moves = this.getKnightMoves(row, col); break;
            case 'b': moves = this.getBishopMoves(row, col); break;
            case 'q': moves = this.getQueenMoves(row, col); break;
            case 'k': moves = this.getKingMoves(row, col); break;
        }

        // Şaha açık bırakan hamleleri filtrele
        return moves.filter(m => !this.wouldBeInCheck(row, col, m.row, m.col));
    }

    getPawnMoves(row, col) {
        const moves = [];
        const dir = this.turn === 'w' ? -1 : 1;
        const startRow = this.turn === 'w' ? 6 : 1;

        // İleri
        if (!this.getPiece(row + dir, col)) {
            moves.push({ row: row + dir, col });
            // Çift adım
            if (row === startRow && !this.getPiece(row + 2 * dir, col)) {
                moves.push({ row: row + 2 * dir, col });
            }
        }

        // Çapraz yeme
        for (const dc of [-1, 1]) {
            const target = this.getPiece(row + dir, col + dc);
            if (col + dc >= 0 && col + dc <= 7) {
                if (this.isEnemyPiece(target)) {
                    moves.push({ row: row + dir, col: col + dc });
                }
                // En passant
                if (this.enPassantTarget &&
                    this.enPassantTarget.row === row + dir &&
                    this.enPassantTarget.col === col + dc) {
                    moves.push({ row: row + dir, col: col + dc, enPassant: true });
                }
            }
        }

        return moves;
    }

    getSlidingMoves(row, col, directions) {
        const moves = [];
        for (const [dr, dc] of directions) {
            let r = row + dr, c = col + dc;
            while (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
                const target = this.getPiece(r, c);
                if (!target) {
                    moves.push({ row: r, col: c });
                } else if (this.isEnemyPiece(target)) {
                    moves.push({ row: r, col: c });
                    break;
                } else {
                    break;
                }
                r += dr;
                c += dc;
            }
        }
        return moves;
    }

    getRookMoves(row, col) {
        return this.getSlidingMoves(row, col, [[0, 1], [0, -1], [1, 0], [-1, 0]]);
    }

    getBishopMoves(row, col) {
        return this.getSlidingMoves(row, col, [[1, 1], [1, -1], [-1, 1], [-1, -1]]);
    }

    getQueenMoves(row, col) {
        return this.getSlidingMoves(row, col, [
            [0, 1], [0, -1], [1, 0], [-1, 0],
            [1, 1], [1, -1], [-1, 1], [-1, -1]
        ]);
    }

    getKnightMoves(row, col) {
        const moves = [];
        const jumps = [[-2, -1], [-2, 1], [-1, -2], [-1, 2], [1, -2], [1, 2], [2, -1], [2, 1]];
        for (const [dr, dc] of jumps) {
            const r = row + dr, c = col + dc;
            if (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
                const target = this.getPiece(r, c);
                if (!target || this.isEnemyPiece(target)) {
                    moves.push({ row: r, col: c });
                }
            }
        }
        return moves;
    }

    getKingMoves(row, col) {
        const moves = [];
        for (let dr = -1; dr <= 1; dr++) {
            for (let dc = -1; dc <= 1; dc++) {
                if (dr === 0 && dc === 0) continue;
                const r = row + dr, c = col + dc;
                if (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
                    const target = this.getPiece(r, c);
                    if (!target || this.isEnemyPiece(target)) {
                        moves.push({ row: r, col: c });
                    }
                }
            }
        }

        // Rok
        if (this.turn === 'w') {
            if (this.castlingRights.K && !this.getPiece(7, 5) && !this.getPiece(7, 6) &&
                this.getPiece(7, 7) === 'R' && !this.isSquareAttacked(7, 4, 'b') &&
                !this.isSquareAttacked(7, 5, 'b') && !this.isSquareAttacked(7, 6, 'b')) {
                moves.push({ row: 7, col: 6, castling: 'K' });
            }
            if (this.castlingRights.Q && !this.getPiece(7, 3) && !this.getPiece(7, 2) &&
                !this.getPiece(7, 1) && this.getPiece(7, 0) === 'R' &&
                !this.isSquareAttacked(7, 4, 'b') && !this.isSquareAttacked(7, 3, 'b') &&
                !this.isSquareAttacked(7, 2, 'b')) {
                moves.push({ row: 7, col: 2, castling: 'Q' });
            }
        } else {
            if (this.castlingRights.k && !this.getPiece(0, 5) && !this.getPiece(0, 6) &&
                this.getPiece(0, 7) === 'r' && !this.isSquareAttacked(0, 4, 'w') &&
                !this.isSquareAttacked(0, 5, 'w') && !this.isSquareAttacked(0, 6, 'w')) {
                moves.push({ row: 0, col: 6, castling: 'k' });
            }
            if (this.castlingRights.q && !this.getPiece(0, 3) && !this.getPiece(0, 2) &&
                !this.getPiece(0, 1) && this.getPiece(0, 0) === 'r' &&
                !this.isSquareAttacked(0, 4, 'w') && !this.isSquareAttacked(0, 3, 'w') &&
                !this.isSquareAttacked(0, 2, 'w')) {
                moves.push({ row: 0, col: 2, castling: 'q' });
            }
        }

        return moves;
    }

    isSquareAttacked(row, col, byColor) {
        // byColor: 'w' veya 'b' — saldıran taraf
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const piece = this.getPiece(r, c);
                if (!piece) continue;
                const isAttacker = byColor === 'w' ? this.isWhitePiece(piece) : this.isBlackPiece(piece);
                if (!isAttacker) continue;

                const type = piece.toLowerCase();
                if (type === 'p') {
                    const dir = this.isWhitePiece(piece) ? -1 : 1;
                    if (r + dir === row && (c - 1 === col || c + 1 === col)) return true;
                } else if (type === 'n') {
                    const jumps = [[-2, -1], [-2, 1], [-1, -2], [-1, 2], [1, -2], [1, 2], [2, -1], [2, 1]];
                    for (const [dr, dc] of jumps) {
                        if (r + dr === row && c + dc === col) return true;
                    }
                } else if (type === 'k') {
                    if (Math.abs(r - row) <= 1 && Math.abs(c - col) <= 1) return true;
                } else {
                    const dirs = [];
                    if (type === 'r' || type === 'q') dirs.push([0, 1], [0, -1], [1, 0], [-1, 0]);
                    if (type === 'b' || type === 'q') dirs.push([1, 1], [1, -1], [-1, 1], [-1, -1]);
                    for (const [dr, dc] of dirs) {
                        let tr = r + dr, tc = c + dc;
                        while (tr >= 0 && tr <= 7 && tc >= 0 && tc <= 7) {
                            if (tr === row && tc === col) return true;
                            if (this.getPiece(tr, tc)) break;
                            tr += dr;
                            tc += dc;
                        }
                    }
                }
            }
        }
        return false;
    }

    wouldBeInCheck(fromRow, fromCol, toRow, toCol) {
        // Geçici hamle yap, şah kontrolü, geri al
        const backup = this.board.map(r => [...r]);
        const piece = this.board[fromRow][fromCol];

        this.board[toRow][toCol] = piece;
        this.board[fromRow][fromCol] = null;

        // En passant yemesi
        if (piece.toLowerCase() === 'p' && fromCol !== toCol && !backup[toRow][toCol]) {
            this.board[fromRow][toCol] = null;
        }

        // Şah pozisyonunu bul
        const kingChar = this.turn === 'w' ? 'K' : 'k';
        let kingRow = -1, kingCol = -1;
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                if (this.board[r][c] === kingChar) { kingRow = r; kingCol = c; break; }
            }
            if (kingRow !== -1) break;
        }

        const enemy = this.turn === 'w' ? 'b' : 'w';
        const inCheck = this.isSquareAttacked(kingRow, kingCol, enemy);

        // Tahtayı geri yükle
        this.board = backup;
        return inCheck;
    }

    isInCheck() {
        const kingChar = this.turn === 'w' ? 'K' : 'k';
        let kingRow = -1, kingCol = -1;
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                if (this.board[r][c] === kingChar) { kingRow = r; kingCol = c; break; }
            }
            if (kingRow !== -1) break;
        }
        const enemy = this.turn === 'w' ? 'b' : 'w';
        return this.isSquareAttacked(kingRow, kingCol, enemy);
    }

    hasLegalMoves() {
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const piece = this.getPiece(r, c);
                if (piece && this.isOwnPiece(piece)) {
                    if (this.getValidMoves(r, c).length > 0) return true;
                }
            }
        }
        return false;
    }

    makeMove(fromRow, fromCol, toRow, toCol) {
        const piece = this.board[fromRow][fromCol];
        if (!piece) return false;

        const validMoves = this.getValidMoves(fromRow, fromCol);
        const moveInfo = validMoves.find(m => m.row === toRow && m.col === toCol);
        if (!moveInfo) return false;

        // Hamle kaydı (undo için)
        const moveRecord = {
            from: { row: fromRow, col: fromCol },
            to: { row: toRow, col: toCol },
            piece,
            captured: this.board[toRow][toCol],
            castlingRights: { ...this.castlingRights },
            enPassantTarget: this.enPassantTarget ? { ...this.enPassantTarget } : null,
            halfMoveClock: this.halfMoveClock,
            enPassantCapture: null,
            promotedFrom: null
        };

        // En passant yemesi
        if (moveInfo.enPassant) {
            moveRecord.enPassantCapture = { row: fromRow, col: toCol, piece: this.board[fromRow][toCol] };
            this.board[fromRow][toCol] = null;
        }

        // Rok
        if (moveInfo.castling) {
            if (moveInfo.castling === 'K' || moveInfo.castling === 'k') {
                const rookRow = moveInfo.castling === 'K' ? 7 : 0;
                this.board[rookRow][5] = this.board[rookRow][7];
                this.board[rookRow][7] = null;
                moveRecord.castlingType = moveInfo.castling;
            } else {
                const rookRow = moveInfo.castling === 'Q' ? 7 : 0;
                this.board[rookRow][3] = this.board[rookRow][0];
                this.board[rookRow][0] = null;
                moveRecord.castlingType = moveInfo.castling;
            }
        }

        // Taşı taşı
        this.board[toRow][toCol] = piece;
        this.board[fromRow][fromCol] = null;

        // Piyon terfisi (otomatik vezir)
        const promoteRow = this.turn === 'w' ? 0 : 7;
        if (piece.toLowerCase() === 'p' && toRow === promoteRow) {
            moveRecord.promotedFrom = piece;
            this.board[toRow][toCol] = this.turn === 'w' ? 'Q' : 'q';
        }

        // En passant hedefi güncelle
        this.enPassantTarget = null;
        if (piece.toLowerCase() === 'p' && Math.abs(toRow - fromRow) === 2) {
            this.enPassantTarget = { row: (fromRow + toRow) / 2, col: fromCol };
        }

        // Rok hakları güncelle
        if (piece === 'K') { this.castlingRights.K = false; this.castlingRights.Q = false; }
        if (piece === 'k') { this.castlingRights.k = false; this.castlingRights.q = false; }
        if (piece === 'R' && fromRow === 7 && fromCol === 7) this.castlingRights.K = false;
        if (piece === 'R' && fromRow === 7 && fromCol === 0) this.castlingRights.Q = false;
        if (piece === 'r' && fromRow === 0 && fromCol === 7) this.castlingRights.k = false;
        if (piece === 'r' && fromRow === 0 && fromCol === 0) this.castlingRights.q = false;

        // Kaleyi yiyen hamleler de rok hakkını kaldırır
        if (toRow === 0 && toCol === 7) this.castlingRights.k = false;
        if (toRow === 0 && toCol === 0) this.castlingRights.q = false;
        if (toRow === 7 && toCol === 7) this.castlingRights.K = false;
        if (toRow === 7 && toCol === 0) this.castlingRights.Q = false;

        // Sıra değişimi
        this.turn = this.turn === 'w' ? 'b' : 'w';
        if (this.turn === 'w') this.fullMoveNumber++;

        // Son hamle
        this.lastMove = { from: { row: fromRow, col: fromCol }, to: { row: toRow, col: toCol } };

        this.moveHistory.push(moveRecord);

        // Oyun sonu kontrolü
        if (!this.hasLegalMoves()) {
            this.gameOver = true;
            this.gameResult = this.isInCheck() ? 'checkmate' : 'stalemate';
        }

        return true;
    }

    undoMove() {
        if (this.moveHistory.length === 0) return false;

        const move = this.moveHistory.pop();

        // Sıra geri al
        this.turn = this.turn === 'w' ? 'b' : 'w';
        if (this.turn === 'b') this.fullMoveNumber--;

        // Terfi geri al
        const restorePiece = move.promotedFrom || move.piece;

        // Taşı geri koy
        this.board[move.from.row][move.from.col] = restorePiece;
        this.board[move.to.row][move.to.col] = move.captured;

        // En passant geri al
        if (move.enPassantCapture) {
            this.board[move.enPassantCapture.row][move.enPassantCapture.col] = move.enPassantCapture.piece;
        }

        // Rok geri al
        if (move.castlingType) {
            if (move.castlingType === 'K') {
                this.board[7][7] = this.board[7][5];
                this.board[7][5] = null;
            } else if (move.castlingType === 'Q') {
                this.board[7][0] = this.board[7][3];
                this.board[7][3] = null;
            } else if (move.castlingType === 'k') {
                this.board[0][7] = this.board[0][5];
                this.board[0][5] = null;
            } else if (move.castlingType === 'q') {
                this.board[0][0] = this.board[0][3];
                this.board[0][3] = null;
            }
        }

        // Durum geri al
        this.castlingRights = move.castlingRights;
        this.enPassantTarget = move.enPassantTarget;
        this.halfMoveClock = move.halfMoveClock;

        // Son hamle güncelle
        this.lastMove = this.moveHistory.length > 0
            ? { from: this.moveHistory[this.moveHistory.length - 1].from, to: this.moveHistory[this.moveHistory.length - 1].to }
            : null;

        this.gameOver = false;
        this.gameResult = '';

        return true;
    }

    // Geçerli pozisyonun FEN string'i (ileride motor iletişimi için)
    toFEN() {
        let fen = '';

        for (let r = 0; r < 8; r++) {
            let empty = 0;
            for (let c = 0; c < 8; c++) {
                const piece = this.board[r][c];
                if (!piece) {
                    empty++;
                } else {
                    if (empty > 0) { fen += empty; empty = 0; }
                    fen += piece;
                }
            }
            if (empty > 0) fen += empty;
            if (r < 7) fen += '/';
        }

        fen += ' ' + this.turn;

        let castling = '';
        if (this.castlingRights.K) castling += 'K';
        if (this.castlingRights.Q) castling += 'Q';
        if (this.castlingRights.k) castling += 'k';
        if (this.castlingRights.q) castling += 'q';
        fen += ' ' + (castling || '-');

        if (this.enPassantTarget) {
            const files = 'abcdefgh';
            const ranks = '87654321';
            fen += ' ' + files[this.enPassantTarget.col] + ranks[this.enPassantTarget.row];
        } else {
            fen += ' -';
        }

        fen += ' ' + this.halfMoveClock;
        fen += ' ' + this.fullMoveNumber;

        return fen;
    }

    getMoveNotation(fromRow, fromCol, toRow, toCol) {
        const files = 'abcdefgh';
        const ranks = '87654321';
        return files[fromCol] + ranks[fromRow] + files[toCol] + ranks[toRow];
    }
}
