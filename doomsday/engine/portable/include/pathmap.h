/**\file pathmap.h
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2011-2012 Daniel Swanson <danij@dengine.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef LIBDENG_FILESYS_PATHMAP_H
#define LIBDENG_FILESYS_PATHMAP_H

/**
 * Path fragment info.
 */
typedef struct pathmapfragment_s {
    ushort hash;
    const char* from, *to;
    struct pathmapfragment_s* next;
} PathMapFragment;

/**
 * PathMap. Can be allocated on the stack.
 */
/// Size of the fixed-length "short" path (in characters) allocated with the map.
#define PATHMAP_SHORT_PATH 256

/// Size of the fixed-length "short" fragment buffer allocated with the map.
#define PATHMAP_FRAGMENTBUFFER_SIZE 8

typedef struct {
    char _shortPath[PATHMAP_SHORT_PATH+1];
    char* _path; // The long version.
    char _delimiter;

    /// Total number of fragments in the search term.
    uint _fragmentCount;

    /**
     * Fragment map of the search term. The map is split into two components.
     * The first PATHMAP_FRAGMENTBUFFER_SIZE elements are placed
     * into a fixed-size buffer allocated along with "this". Any additional fragments
     * are attached to "this" using a linked list.
     *
     * This optimized representation hopefully means that the majority of searches
     * can be fulfilled without dynamically allocating memory.
     */
    PathMapFragment _fragmentBuffer[PATHMAP_FRAGMENTBUFFER_SIZE];

    /// Head of the linked list of "extra" fragments, in reverse order.
    PathMapFragment* _extraFragments;
} PathMap;

/**
 * Initialize the specified PathMap from the given path.
 *
 * \post The path will have been subdivided into a fragment map and some or
 * all of the fragment hashes will have been calculated (dependant on the
 * number of discreet fragments).
 *
 * @param path  Relative or absolute path to be mapped.
 * @param delimiter  Fragments of @a path are delimited by this character.
 * @return  Pointer to "this" instance for caller convenience.
 */
PathMap* PathMap_Initialize(PathMap* pathMap, const char* path, char delimiter);

/**
 * Destroy @a pathMap releasing any resources acquired for it.
 */
void PathMap_Destroy(PathMap* pathMap);

/// @return  Number of fragments in the mapped path.
uint PathMap_Size(PathMap* pathMap);

/**
 * Retrieve the info for fragment @a idx within the path. Note that fragments
 * are indexed in reverse order (compared to the logical, left-to-right
 * order of the original path).
 *
 * For example, if the path is "c:/mystuff/myaddon.addon" the corresponding
 * fragment map will be: [0:{myaddon.addon}, 1:{mystuff}, 2:{c:}].
 *
 * \post Hash may have been calculated for the referenced fragment.
 *
 * @param idx  Reverse-index of the fragment to be retrieved.
 * @return  Processed fragment info else @c NULL if @a idx is invalid.
 */
const PathMapFragment* PathMap_Fragment(PathMap* pathMap, uint idx);

#endif /* LIBDENG_FILESYS_PATHMAP_H */
