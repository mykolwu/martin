/*
 * This file contains the implementation of calculating a stalemate based off a randomly generated
 * opponent king location and set of pieces
 */
#include "stalemate.h"
#include "testing/SimpleTest.h"

using namespace std;

/* This function takes in a GridLocation and Vector of characters and returns a map of pieces to a
 * location that achieves stalemate. It greedily gets possible locations and recursively tests
 * combinations.
 */
Map<char, Vector<GridLocation>> calculateStalemate(GridLocation kingLoc, Vector<char> pieces) {
    Set<GridLocation> adjacentLocs = getAdjacentLocs(kingLoc);
    Set<GridLocation> exclusion = adjacentLocs;
    Map<char, Vector<GridLocation>> result;
    Map<char, Vector<GridLocation>> pieceBestLocs;

    for (char i : pieces) {
        pieceBestLocs[i] = greedyHelper(i, adjacentLocs);
    }

    placePieceGreedy(pieces, 0, pieceBestLocs, exclusion, result, kingLoc);

    calculateExclusion(exclusion, kingLoc, result);
    removeUsedPieces(pieces, result);
    placeUselessPieces(pieces, exclusion, kingLoc, result);

    return result;
}

/* This function more optimally takes in a GridLocation and Vector of characters and returns a map of pieces
 * to their locations. It adds in pre-sorting to calculate most powerful pieces first.
 */
Map<char, Vector<GridLocation>> calculateStalemateAlternative(GridLocation kingLoc, Vector<char> pieces) {
    Set<GridLocation> adjacentLocs = getAdjacentLocs(kingLoc);
    Set<GridLocation> exclusion = adjacentLocs;
    Map<char, Vector<GridLocation>> result;
    Map<char, Vector<GridLocation>> pieceBestLocs;

    sort(pieces);

    for (char i : pieces) {
        pieceBestLocs[i] = greedyHelper(i, adjacentLocs);
    }

    Set<GridLocation> takenLocs;
    placePieceGreedy(pieces, 0, pieceBestLocs, exclusion, result, kingLoc);

    calculateExclusion(exclusion, kingLoc, result);
    removeUsedPieces(pieces, result);
    placeUselessPieces(pieces, exclusion, kingLoc, result);

    return result;
}

/* This function takes in a Vector of characters by reference and sorts it based on piece power (Queen, Rook, Bishop, Knight) with King at front.
 */
void sort(Vector<char> &pieces) {
    char king = pieces.remove(0);
    pieces.sort();
    pieces.reverse();

    int index = pieces.indexOf('Q');
    for (int i = 0; i < index && index != -1; i++) {
        pieces[index + i] = 'R';
        pieces[i] = 'Q';
    }
    pieces.insert(0, king);
}

/* This function takes in a Vector of characters and a Map of characters to Vector of GridLocations by reference and removes the pieces used
 * in the result map from the vector.
 */
void removeUsedPieces(Vector<char> &pieces, Map<char, Vector<GridLocation>> &result) {
    pieces.sort();
    for (char i : result.keys()) {
        int index = pieces.indexOf(i);
        for (int j = 0; j < result[i].size(); j++) {
            pieces.remove(index);
        }
    }
}

/* This function takes in a character piece, GridLocation for that piece, and opponent king location and returns a boolean
 * on whether the piece is attacking the king at its location using set operators.
 */
bool notAttackingKing(char piece, GridLocation loc, GridLocation kingLoc) {
    Set<GridLocation> adjacents = getAdjacentLocs(kingLoc);
    Set<GridLocation> attacking = pieceAttackingLocs(piece, loc);
    return adjacents == adjacents - attacking;
}

/* This function takes in a Vector of characters, Set of GridLocations for excluded locations, opponent king location, and
 * a Map of characters to Vector of GridLocations by reference. It places the remaining the pieces in benevolent locations
 * (not attacking the opponent king) and adding those characters and locations to the result map.
 */
void placeUselessPieces(Vector<char> pieces, Set<GridLocation> exclusion, GridLocation kingLoc, Map<char, Vector<GridLocation>> &result) {
    if (pieces.size() == 0) {
        return;
    }
    char piece = pieces.remove(0);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (notAttackingKing(piece, GridLocation(i, j), kingLoc) && !exclusion.contains(GridLocation(i, j))) {
                result[piece].add(GridLocation(i, j));
                _board[GridLocation(i, j)] = piece;
                if (pieces.size() == 0) {
                    return;
                }
                piece = pieces.remove(0);
            }
        }
    }
}

/* This function takes in a Set of excluded Gridlocations by reference, opponent king location, and Map of characters to Vector of
 * GridLocations. It calculates the locations pieces are occupying and stores it in the Set.
 */
void calculateExclusion(Set<GridLocation> &exclusionLocs, GridLocation kingLoc, Map<char, Vector<GridLocation>> result) {
    exclusionLocs = getAdjacentLocs(kingLoc);
    for (char i : result.keys()) {
        for (GridLocation j : result[i]) {
            exclusionLocs.add(j);
        }
    }
}

/* This function takes in a Vector of characters by reference, integer index, optimal move Map of characters to Vector of GridLocations,
 * Set of excluded GridLocations by reference, result Map of characters to Vector of GridLocations by reference, and opponent king location.
 * It recursively tests combinations of pieces and their locations by incrementing the index to move to the next piece, considering each
 * optimal move for each piece. It returns true when a stalemate is achieved or false when all combinations are exhuasted.
 */
bool placePieceGreedy(Vector<char> &pieces, int pieceIndex, Map<char, Vector<GridLocation>> &moves,
                      Set<GridLocation> &exclusionLocs, Map<char, Vector<GridLocation>> &result,
                      GridLocation kingLoc) {
    if (isStalemate(kingLoc, result)) return true;

    if (pieceIndex > pieces.size() - 1) return false;

    for (GridLocation loc : moves[pieces[pieceIndex]]) {
        if (!exclusionLocs.contains(loc)) {
            _board[loc] = pieces[pieceIndex];
            result[pieces[pieceIndex]].add(loc);
            exclusionLocs.add(loc);

            if (placePieceGreedy(pieces, pieceIndex + 1, moves, exclusionLocs, result, kingLoc)) return true;

            _board[loc] = 'E';
            result[pieces[pieceIndex]].remove(result[pieces[pieceIndex]].size() - 1);
        }
    }
    calculateExclusion(exclusionLocs, kingLoc, result);
    return false;
}

/* This function takes in a character piece, GridLocation of the piece, and Set of adjacent GridLocations of opponents king.
 * It returns the number of adjacent locations of the opponents king that the piece is attacking on its location.
 */
int numAttackingAdjacent(char piece, GridLocation loc, Set<GridLocation> &adjacents) {
    return (adjacents.size() - (adjacents - pieceAttackingLocs(piece, loc)).size());
}

/* This function takes in a character and Set of excluded GridLocations by reference. It returns a vector of optimal GridLocations
 * for the piece by maximizing the number of adjacent locations of the opponent king attacked by the piece.
 */
Vector<GridLocation> greedyHelper(char piece, Set<GridLocation> &adjacents) {
    int best = 0;
    Vector<GridLocation> result;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (!adjacents.contains(GridLocation(i, j))) {
                int n = numAttackingAdjacent(piece, GridLocation(i, j), adjacents);
                if (n > best) {
                    result.clear();
                    result.add(GridLocation(i, j));
                    best = n;
                } else if (n == best) {
                    result.add(GridLocation(i, j));
                }
            }
        }
    }
    return result;
}

/* This function takes a GridLocation and returns the 8 adjacent GridLocations in addition to the GridLocation itself.
 */
Set<GridLocation> getAdjacentLocs(GridLocation loc) {
    Set<GridLocation> adjacents;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (_board.inBounds(GridLocation(loc.row + i, loc.col + j))) {
                adjacents.add(GridLocation(loc.row + i, loc.col + j));
            }
        }
    }
    return adjacents;
}

/* This function takes in a Set of adjacent GridLocations for the opponent king, a character piece, and the piece's GridLocation.
 * It removes the adjacent locations attacked by the piece and the location.
 */
void removeAttackedLocs(Set<GridLocation> &kingAdjacentLocs, char piece, GridLocation pieceLoc) {
    Set<GridLocation> attackedLocs = pieceAttackingLocs(piece, pieceLoc);
    kingAdjacentLocs = kingAdjacentLocs - attackedLocs;
}

/* This function takes in a Set of GridLocations by reference and the GridLocation of a piece. It adds the horizontal and vertical
 * locations from the piece's GridLocation to the Set, given that there is no piece obstructing the horizontal or vertical direction
 * from the piece's GridLocations.
 */
void rowAttackingLocs(Set<GridLocation> &locs, GridLocation pieceLoc) {
    GridLocation loc;
    Vector<Vector<int>> dirs = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    for (Vector<int> dir : dirs) {
        loc = GridLocation(pieceLoc.row + dir[0], pieceLoc.col + dir[1]);
        while (_board.inBounds(loc) && _board[loc] == 'E') {
            locs.add(loc);
            loc = GridLocation(loc.row + dir[0], loc.col + dir[1]);
        }
    }
}

/* This function takes in a Set of GridLocations by reference and the GridLocation of a piece. It adds the diagonal
 * locations from the piece's GridLocation to the Set, given that there is no piece obstructing the diagonal direction from
 * the piece's GridLocation.
 */
void diagonalAttackingLocs(Set<GridLocation> &locs, GridLocation pieceLoc) {
    GridLocation loc;
    Vector<Vector<int>> dirs = {{1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
    for (Vector<int> j : dirs) {
        loc = GridLocation(pieceLoc.row + j[0], pieceLoc.col + j[1]);
        while (_board.inBounds(loc) && _board[loc] == 'E') {
            locs.add(loc);
            loc = GridLocation(loc.row + j[0], loc.col + j[1]);
        }
    }
}

/* This function takes in a character piece and the GridLocation for that piece and returns a Set of GridLocations that
 * are attacked by the piece.
 */
Set<GridLocation> pieceAttackingLocs(char piece, GridLocation pieceLoc) {
    Set<GridLocation> locs;
    GridLocation loc;
    switch (piece) {
        case 'K': {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    loc = GridLocation(pieceLoc.row + i, pieceLoc.col + j);
                    if (_board.inBounds(loc) && _board[loc] == 'E' && loc != pieceLoc) {
                        locs.add(loc);
                    }
                }
            }
            break;
        }
        case 'Q': {
            rowAttackingLocs(locs, pieceLoc);
            diagonalAttackingLocs(locs, pieceLoc);
            break;
        }
        case 'R': {
            rowAttackingLocs(locs, pieceLoc);
            break;
        }
        case 'H': {
            Vector<Vector<int>> dirs = {{1, 2}, {2, 1}, {1, -2}, {2, -1}, {-1, 2}, {-2, 1}, {-1, -2}, {-2, -1}};
            for (Vector<int> j : dirs) {
                if (_board.inBounds(GridLocation(pieceLoc.row + j[0], pieceLoc.col + j[1]))) {
                    locs.add(GridLocation(pieceLoc.row + j[0], pieceLoc.col + j[1]));
                }
            }
            break;
        }
        case 'B': {
            diagonalAttackingLocs(locs, pieceLoc);
            break;
        }
        default: {
            error("Invalid character representation of a piece");
        }
    }
    return locs;
}

/* This function takes in an integer and generates and returns a random Vector of characters in which a stalemate is always possible. The
 * number of pieces ranges from 2 to the passed in integer, not including the king.
 */
Vector<char> generatePieces(int max) {
    string possiblePieces = "BHRQ";
    int n = randomInteger(2, max);
    Vector<char> result;
    int numQ = 0;

    if (n == 2) {
        Vector<Vector<char>> twoPieces = {{'K', 'Q', 'Q'}, {'K', 'Q', 'B'}};
        return twoPieces[randomInteger(0, 1)];
    }

    if (n == 3) {
        Vector<Vector<char>> threePieces = {{'K', 'R', 'R'}, {'K', 'B', 'B'}, {'K', 'H', 'H'}, {'K', 'H', 'B'}};
        int choice = randomInteger(0, 3);
        result = threePieces[choice];
        if (choice == 0) {
            result.add(possiblePieces[randomInteger(0, 3)]);
            return result;
        }
        result.add(possiblePieces[randomInteger(2, 3)]);
        return result;
    }

    result.add('K');
    int random;
    for (int i = 0; i < n; i++) {
        if (numQ >= 5) {
            random = randomInteger(0, 2);
        } else {
            random = randomInteger(0, 3);
        }
        if (random == 3) {
            numQ += 1;
        }
        result.add(possiblePieces[random]);
    }
    return result;
}

/* This function takes in the opponent king GridLocation and a Map of characters to GridLocations and returns a boolean
 * on whether a stalemate has been achieved.
 */
bool isStalemate(GridLocation kingLoc, Map<char, Vector<GridLocation>> pieceLocs) {
    Set<GridLocation> adjacentLocs = getAdjacentLocs(kingLoc);
    for (char i : pieceLocs.keys()) {
        for (GridLocation j : pieceLocs[i]) {
            removeAttackedLocs(adjacentLocs, i, j);
        }

    }
    return adjacentLocs.size() == 1 && adjacentLocs.contains(kingLoc);
}

/* This function initialized the board with the opponent king and returns the GridLocation of the randomly generated opponent king location.
 */
GridLocation initializeBoard() {
    _board = Grid<char>(8, 8, 'E');
    int randomRow = randomInteger(1, 6);
    int randomCol = randomInteger(1, 6);
    _board[randomRow][randomCol] = 'K';
    return GridLocation(randomRow, randomCol);
}

/* This function resets the board to all empty squares.
 */
void clearBoard() {
    _board = Grid<char>(8, 8, 'E');
}

/* This function takes in a Graphics window and GridLocation for the opponent king and draws a 8x8 grid with the pieces in their corresponding
 * locations.
 */
void visualizeBoard(GWindow &window, GridLocation kingLoc) {
    window.setSize(1000, 1000);
    GRectangle rect = GRectangle(100, 100, 800, 800);
    window.setColor("BLACK");
    window.drawRect(rect);

    int x = 100;
    int y = 100;
    for (int i = 0; i < 9; i++) {
        window.drawLine(GPoint(100, y + (i*100)), GPoint(900, y + (i*100)));
        window.drawLine(GPoint(x + (i*100), 100), GPoint(x + (i*100), 900));
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            GPoint point = GPoint(110 + (i*100), 110 + (j*100));
            char piece = _board[GridLocation(i, j)];
            switch(piece) {
            case 'K': {
                if (GridLocation(i, j) == kingLoc) {
                    window.drawImage("res/black-king.png", point.x, point.y);
                } else {
                    window.drawImage("res/white-king.png", point.x, point.y);
                }
                break;
            }
            case 'Q': {
                window.drawImage("res/queen.png", point.x, point.y);
                break;
            }
            case 'R': {
                window.drawImage("res/rook.png", point.x, point.y);
                break;
            }
            case 'B': {
                window.drawImage("res/bishop.png", point.x, point.y);
                break;
            }
            case 'H': {
                window.drawImage("res/horse.png", point.x, point.y);
                break;
            }
            }
        }
    }
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("Initialize board") {
    initializeBoard();
    cout << _board;
    clearBoard();
}

PROVIDED_TEST("Generate pieces") {
    for (int i = 0; i < 10; i++) {
        Vector<char> pieces = generatePieces(10);
        cout << pieces;
    }
}

PROVIDED_TEST("pieceAttackingLocs") {
    Set<GridLocation> locs = pieceAttackingLocs('K', GridLocation(4, 4));
    Set<GridLocation> expectedLocs = {GridLocation(3, 3), GridLocation(3, 4), GridLocation(3, 5),
                                      GridLocation(4, 3), GridLocation(4, 5),
                                      GridLocation(5, 3), GridLocation(5, 4), GridLocation(5, 5)};
    EXPECT_EQUAL(locs, expectedLocs);

    locs = pieceAttackingLocs('K', GridLocation(0, 0));
    expectedLocs = {GridLocation(0, 1), GridLocation(1, 0), GridLocation(1, 1)};
    EXPECT_EQUAL(locs, expectedLocs);

    locs = pieceAttackingLocs('Q', GridLocation(1, 1));
    expectedLocs = {GridLocation(0, 1), GridLocation(2, 1), GridLocation(3, 1),
                    GridLocation(4, 1), GridLocation(5, 1), GridLocation(6, 1), GridLocation(7, 1),
                    GridLocation(1, 0), GridLocation(1, 2), GridLocation(1, 3),
                    GridLocation(1, 4), GridLocation(1, 5), GridLocation(1, 6), GridLocation(1, 7),
                    GridLocation(0, 0), GridLocation(2, 2), GridLocation(3, 3),
                    GridLocation(4, 4), GridLocation(5, 5), GridLocation(6, 6), GridLocation(7, 7),
                    GridLocation(2, 0), GridLocation(0, 2)
                   };
    EXPECT_EQUAL(locs, expectedLocs);

    locs = pieceAttackingLocs('R', GridLocation(0, 1));
    expectedLocs = {GridLocation(1, 1), GridLocation(2, 1), GridLocation(3, 1),
                    GridLocation(4, 1), GridLocation(5, 1), GridLocation(6, 1), GridLocation(7, 1),
                    GridLocation(0, 0), GridLocation(0, 2), GridLocation(0, 3),
                    GridLocation(0, 4), GridLocation(0, 5), GridLocation(0, 6), GridLocation(0, 7),
                   };
    EXPECT_EQUAL(locs, expectedLocs);

    locs = pieceAttackingLocs('B', GridLocation(2, 3));
    expectedLocs = {GridLocation(0, 1), GridLocation(1, 2), GridLocation(3, 4),
                    GridLocation(4, 5), GridLocation(5, 6), GridLocation(6, 7), GridLocation(3, 2),
                    GridLocation(4, 1), GridLocation(5, 0), GridLocation(1, 4), GridLocation(0, 5),
                   };
    EXPECT_EQUAL(locs, expectedLocs);

    locs = pieceAttackingLocs('H', GridLocation(4, 4));
    expectedLocs = {GridLocation(3, 2), GridLocation(2, 3), GridLocation(5, 2), GridLocation(6, 3),
                    GridLocation(2, 5), GridLocation(3, 6), GridLocation(6, 5), GridLocation(5, 6),
                   };
    EXPECT_EQUAL(locs, expectedLocs);
    clearBoard();
}

PROVIDED_TEST("getAdjacentLocs") {
    Set<GridLocation> adjacent = getAdjacentLocs(GridLocation(0, 0));
    Set<GridLocation> expected = {GridLocation(1, 0), GridLocation(0, 1), GridLocation(1, 1), GridLocation(0, 0)};
    EXPECT_EQUAL(adjacent, expected);

    adjacent = getAdjacentLocs(GridLocation(4, 4));
    expected = {GridLocation(3, 3), GridLocation(3, 4), GridLocation(3, 5), GridLocation(4, 3),
                GridLocation(4, 5), GridLocation(5, 3), GridLocation(5, 4), GridLocation(5, 5),
                GridLocation(4, 4)};
    EXPECT_EQUAL(adjacent, expected);

    adjacent = getAdjacentLocs(GridLocation(2, 0));
    expected = {GridLocation(2, 1), GridLocation(1, 0), GridLocation(1, 1), GridLocation(3, 0),
                GridLocation(3, 1), GridLocation(2, 0)
               };
    EXPECT_EQUAL(adjacent, expected);
    clearBoard();
}

PROVIDED_TEST("removeAttackedLocs") {
    GridLocation kingLoc = GridLocation(1, 1);
    Set<GridLocation> adjacentLocs = getAdjacentLocs(kingLoc);
    removeAttackedLocs(adjacentLocs, 'Q', GridLocation(3, 0));
    Set<GridLocation> expectedRemainingLocs = {GridLocation(0, 1), GridLocation(0, 2), GridLocation(2, 2), GridLocation(1, 1)};
    EXPECT_EQUAL(adjacentLocs, expectedRemainingLocs);

    removeAttackedLocs(adjacentLocs, 'K', GridLocation(1, 3));
    expectedRemainingLocs = {GridLocation(0, 1), GridLocation(1, 1)};
    EXPECT_EQUAL(adjacentLocs, expectedRemainingLocs);
}

PROVIDED_TEST("isStalemate") {
    EXPECT(isStalemate(GridLocation(1, 1), {{'Q', {GridLocation(3, 0)}}, {'K', {GridLocation(1, 3)}}, {'B', {GridLocation(3, 4)}}}));
}

PROVIDED_TEST("greedyHelp") {
    Set<GridLocation> adjacents = getAdjacentLocs(GridLocation(1, 1));
    Vector<GridLocation> bestLocs = greedyHelper('K', adjacents);
    cout << bestLocs;
}

PROVIDED_TEST("isStalemate") {
    EXPECT(!isStalemate(GridLocation(1, 1), {{'Q', {GridLocation(3, 0)}}, {'K', {GridLocation(1, 3)}}, {'Q', {GridLocation(2, 3)}}}));
}

PROVIDED_TEST("placeUselessPieces") {
    initializeBoard();
    clearBoard();
    GridLocation kingLoc = GridLocation (1, 1);
    _board[kingLoc] = 'K';
    Vector<char> pieces = {'R', 'R', 'Q', 'H', 'Q'};
    Set<GridLocation> adjacents = getAdjacentLocs(kingLoc);
    Map<char, Vector<GridLocation>> result = {{'Q', {GridLocation(3, 0), GridLocation(2, 3)}}, {'K', {GridLocation(1, 3)}}};
    Set<GridLocation> exclusion;
    calculateExclusion(exclusion, kingLoc, result);
    placeUselessPieces(pieces, exclusion, kingLoc, result);
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("removedUsedPieces") {
    initializeBoard();
    Vector<char> pieces = {'R', 'R', 'Q', 'H', 'Q', 'K'};
    Map<char, Vector<GridLocation>> result = {{'Q', {GridLocation(3, 0), GridLocation(2, 3)}}, {'K', {GridLocation(1, 3)}}};
    removeUsedPieces(pieces, result);
    EXPECT(pieces.equals({'H', 'R', 'R'}));
    clearBoard();
}

PROVIDED_TEST("calculateStalemate with two Queens") {
    clearBoard();
    _board[GridLocation(1, 1)] = 'K';
    GridLocation kingLoc = GridLocation(1, 1);
    Map<char, Vector<GridLocation>> result = calculateStalemate(GridLocation(1, 1), {'K', 'Q', 'Q'});
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("calculateStalemate with Queen and Bishop") {
    clearBoard();
    _board[GridLocation(1, 1)] = 'K';
    GridLocation kingLoc = GridLocation(1, 1);
    Map<char, Vector<GridLocation>> result = calculateStalemate(GridLocation(1, 1), {'K', 'Q', 'B'});
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("calculateStalemateAlternative with large number of pieces not in order") {
    clearBoard();
    GridLocation kingLoc = GridLocation(2, 1);
    Vector<char> pieces = {'K', 'H', 'B', 'Q', 'R', 'Q', 'Q', 'Q', 'H', 'H', 'H'};
    Map<char, Vector<GridLocation>> result = calculateStalemateAlternative(kingLoc, pieces);
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("calculateStalemate with large number of pieces in order") {
    clearBoard();
    GridLocation kingLoc = GridLocation(2, 1);
    Vector<char> pieces = {'K', 'R', 'Q', 'Q', 'Q', 'Q', 'B', 'H', 'H', 'H', 'H'};
    Map<char, Vector<GridLocation>> result = calculateStalemate(kingLoc, pieces);
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("calculateStalemate with max number of queens") {
    clearBoard();
    GridLocation kingLoc = GridLocation(2, 1);
    Vector<char> pieces = {'K', 'Q', 'Q', 'Q', 'Q', 'Q', 'R', 'R', 'R', 'R', 'R'};
    Map<char, Vector<GridLocation>> result = calculateStalemate(kingLoc, pieces);
    EXPECT(isStalemate(kingLoc, result));
    clearBoard();
}

PROVIDED_TEST("Sort") {
    Vector<char> pieces = {'K', 'R', 'Q', 'B', 'R', 'Q', 'B', 'H', 'Q', 'R', 'H'};
    sort(pieces);
    EXPECT(pieces.equals({'K', 'Q', 'Q', 'Q', 'R', 'R', 'R', 'H', 'H', 'B', 'B'}));

    pieces = {'K', 'Q'};
    sort(pieces);
    EXPECT(pieces.equals({'K', 'Q'}));

    pieces = {'K', 'R'};
    EXPECT(pieces.equals({'K', 'R'}));
    sort(pieces);

    pieces = {'K', 'H'};
    EXPECT(pieces.equals({'K', 'H'}));
    sort(pieces);

    pieces = {'K', 'B'};
    EXPECT(pieces.equals({'K', 'B'}));
    sort(pieces);
}

PROVIDED_TEST("calculateStalemate with randomly generated opponent king and pieces") {
    GridLocation kingLoc = initializeBoard();
    Vector<char> pieces = generatePieces(5); // less pieces to avoid taking too long
    Map<char, Vector<GridLocation>> result = calculateStalemate(kingLoc, pieces);
    EXPECT(isStalemate(kingLoc, result));

    GWindow window;
    visualizeBoard(window, kingLoc);
    clearBoard();
}

PROVIDED_TEST("calculateStalemate with randomly generated opponent king and pieces") {
    GridLocation kingLoc = initializeBoard();
    Vector<char> pieces = generatePieces(10);
    Map<char, Vector<GridLocation>> result = calculateStalemateAlternative(kingLoc, pieces);
    EXPECT(isStalemate(kingLoc, result));

    GWindow window;
    visualizeBoard(window, kingLoc);
    clearBoard();
}
