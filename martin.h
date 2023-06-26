/* 
 * This file contains the function declarations for calculate a stalemate off of a randomly generated opponent
 * king location and set of pieces
 */
#pragma once

#include "grid.h"
#include "map.h"
#include "set.h"
#include "gtypes.h"
#include "gwindow.h"

/** Global board variable
 */
Grid<char> _board;

/**
 * Calculates a stalemate position
 * @param opponent king location and random set of pieces
 * @return map of pieces to their locations to achieve a stalemate
 *
 * This function runs in O(k^n) with k being the number of possible moves for a piece, and n being the number of pieces.
 */


Map<char, Vector<GridLocation>> calculateStalemate(GridLocation kingLoc, Vector<char> pieces);


/**
 * More efficient way to calculate stalemate position with pre-sorting
 * @param opponent king location and random set of pieces
 * @return map of pieces to their locations to achieve a stalemate
 *
 * This function also runs in O(k^n)
 */
Map<char, Vector<GridLocation>> calculateStalemateAlternative(GridLocation kingLoc, Vector<char> pieces);

/**
 * Sort pieces from most to least efficient (Queen, Rook, Bishop, Knight) while keeping King at the front
 * @param pieces
 *
 * This function runs in O(n) with n being number of bishops
 */
void sort(Vector<char> &pieces);

/**
 * Remove pieces used in stalemate
 * @param pieces and resulting map for stalemate
 *
 * This function runs in O(nk) with n being different pieces in used in result map and k being number of said piece
 */
void removeUsedPieces(Vector<char> &pieces, Map<char, Vector<GridLocation>> &result);

/**
 * Place remaining pieces not used in result map
 * @param remaining pieces, locations that are taken (should be excluded), opponent king location, and result map
 *
 * This function runs in O(n) for n pieces
 */
void placeUselessPieces(Vector<char> pieces, Set<GridLocation> exclusion, GridLocation kingLoc, Map<char, Vector<GridLocation>> &result);

/**
 * Recalculate locations that are already taken
 * @param exclusion set, opponent king location, and result map
 *
 * This function runs in O(nk) for n unique pieces and k number of piece in result map
 */
void calculateExclusion(Set<GridLocation> &exclusionLocs, GridLocation kingLoc, Map<char, Vector<GridLocation>> result);

/**
 * Place pieces on optimal squares with all possible combinations
 * @param pieces, index of current piece, map of pieces to optimal moves, taken locations, result map, and opponent king location
 * @return true for pieces placed achieve stalemate
 *
 * This function runs in O(k^n) for k optimal moves for n pieces
 */
bool placePieceGreedy(Vector<char> &pieces, int pieceIndex, Map<char, Vector<GridLocation>> &moves, Set<GridLocation> &exclusionLocs,
                      Map<char, Vector<GridLocation>> &result, GridLocation kingLoc);

/**
 * Helper function for getting optimal moves that take the most squares away from the opponents king
 * @param piece, adjacent locations of opponent king
 * @return vector of optimal moves
 *
 * This function runs in O(1)
 */
Vector<GridLocation> greedyHelper(char piece, Set<GridLocation> &adjacents);

/**
 * Get adjacent locations
 * @param location
 * @return set of locations
 *
 * This function runs in O(1)
 */
Set<GridLocation> getAdjacentLocs(GridLocation loc);

/**
 * Get all locations attacked by given piece
 * @param piece, piece location
 * @return set of locations
 *
 * This function runs in O(1)
 */
Set<GridLocation> pieceAttackingLocs(char piece, GridLocation pieceLoc);

/**
 * Generate random set of pieces where stalemate is always possible
 * @return vector of random pieces
 *
 * This function runs in O(1)
 */
Vector<char> generatePieces();

/**
 * Checks if stalemate is achieved
 * @param opponent king location, map of pieces and their locations
 * @return boolean of stalemate
 *
 * This function runs in O(n) for number of locations
 */
bool isStalemate(GridLocation kingLoc, Map<char, Vector<GridLocation>> pieceLocs);

/**
 * initialize Board
 * @return opponent king location that is randomly generated
 *
 * This function runs in O(1)
 */
GridLocation initializeBoard();

/**
 * Resets board
 *
 * This function runs in O(1)
 */
void clearBoard();

/**
 * Create visual chess board with pieces
 * @param window, opponent king location
 *
 * This function runs in O(1)
 */
void visualizeBoard(GWindow &window, GridLocation kingLoc);
