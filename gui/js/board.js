// board.js — Tahta render ve etkileşim

class ChessBoard {
    constructor(game, container) {
        this.game = game;
        this.container = container;
        this.selectedSquare = null;
        this.validMoveSquares = [];
        this.flipped = false;
        this.onMoveCallback = null;
        this.onSelectCallback = null;
    }

    render() {
        this.container.innerHTML = '';

        // Koordinat çubukları
        const rankLabels = document.createElement('div');
        rankLabels.className = 'rank-labels';
        for (let i = 0; i < 8; i++) {
            const label = document.createElement('span');
            const rank = this.flipped ? i + 1 : 8 - i;
            label.textContent = rank;
            rankLabels.appendChild(label);
        }

        const fileLabels = document.createElement('div');
        fileLabels.className = 'file-labels';
        const files = 'abcdefgh';
        for (let i = 0; i < 8; i++) {
            const label = document.createElement('span');
            const fileIdx = this.flipped ? 7 - i : i;
            label.textContent = files[fileIdx];
            fileLabels.appendChild(label);
        }

        const boardGrid = document.createElement('div');
        boardGrid.className = 'board-grid';

        for (let displayRow = 0; displayRow < 8; displayRow++) {
            for (let displayCol = 0; displayCol < 8; displayCol++) {
                const row = this.flipped ? 7 - displayRow : displayRow;
                const col = this.flipped ? 7 - displayCol : displayCol;

                const square = document.createElement('div');
                const isLight = (row + col) % 2 === 0;
                square.className = 'square ' + (isLight ? 'light' : 'dark');
                square.dataset.row = row;
                square.dataset.col = col;

                // Son hamle highlight
                if (this.game.lastMove) {
                    const lm = this.game.lastMove;
                    if ((row === lm.from.row && col === lm.from.col) ||
                        (row === lm.to.row && col === lm.to.col)) {
                        square.classList.add('last-move');
                    }
                }

                // Seçili kare
                if (this.selectedSquare && this.selectedSquare.row === row && this.selectedSquare.col === col) {
                    square.classList.add('selected');
                }

                // Geçerli hamle noktaları
                const isValidTarget = this.validMoveSquares.some(m => m.row === row && m.col === col);
                if (isValidTarget) {
                    const piece = this.game.getPiece(row, col);
                    if (piece && this.game.isEnemyPiece(piece)) {
                        square.classList.add('capture-target');
                    } else {
                        square.classList.add('valid-target');
                    }
                }

                // Şah kontrolü
                if (this.game.isInCheck()) {
                    const kingChar = this.game.turn === 'w' ? 'K' : 'k';
                    if (this.game.getPiece(row, col) === kingChar) {
                        square.classList.add('in-check');
                    }
                }

                // Taş SVG'si
                const piece = this.game.getPiece(row, col);
                if (piece) {
                    const pieceEl = document.createElement('div');
                    pieceEl.className = 'piece';
                    pieceEl.innerHTML = getPieceSVG(piece);
                    square.appendChild(pieceEl);
                }

                // Tıklama
                square.addEventListener('click', () => this.handleClick(row, col));

                boardGrid.appendChild(square);
            }
        }

        this.container.appendChild(rankLabels);
        this.container.appendChild(boardGrid);
        this.container.appendChild(fileLabels);
    }

    handleClick(row, col) {
        if (this.game.gameOver) return;

        const piece = this.game.getPiece(row, col);

        // Eğer geçerli bir hedef kareye tıkladıysa → hamle yap
        if (this.selectedSquare) {
            const isValid = this.validMoveSquares.some(m => m.row === row && m.col === col);

            if (isValid) {
                const from = this.selectedSquare;
                this.clearSelection();

                if (this.game.makeMove(from.row, from.col, row, col)) {
                    this.render();
                    if (this.onMoveCallback) {
                        this.onMoveCallback(this.game.getMoveNotation(from.row, from.col, row, col));
                    }
                    return;
                }
            }

            // Eğer kendi başka bir taşına tıkladıysa → yeni seçim
            if (piece && this.game.isOwnPiece(piece)) {
                this.selectSquare(row, col);
                return;
            }

            // Geçersiz → seçimi temizle
            this.clearSelection();
            return;
        }

        // İlk tıklama → taş seç
        if (piece && this.game.isOwnPiece(piece)) {
            this.selectSquare(row, col);
        }
    }

    selectSquare(row, col) {
        this.selectedSquare = { row, col };
        this.validMoveSquares = this.game.getValidMoves(row, col);
        this.render();
        if (this.onSelectCallback) this.onSelectCallback(row, col);
    }

    clearSelection() {
        this.selectedSquare = null;
        this.validMoveSquares = [];
        this.render();
    }

    flipBoard() {
        this.flipped = !this.flipped;
        this.render();
    }
}
