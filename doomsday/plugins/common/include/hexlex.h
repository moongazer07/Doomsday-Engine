/** @file hexlex.h  Lexical analyzer for Hexen definition/script syntax.
 *
 * @authors Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright © 2007-2013 Daniel Swanson <danij@dengine.net>
 * @authors Copyright © 1999 Activision
 *
 * @par License
 * GPL: http://www.gnu.org/licenses/gpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA</small>
 */

#ifndef LIBCOMMON_HEXLEX_H
#define LIBCOMMON_HEXLEX_H

#include "common.h"

/**
 * Lexical analyzer for Hexen definition/script syntax.
 */
class HexLex
{
public:
    HexLex();
    ~HexLex();

    /**
     * Prepare a new script for parsing. It is assumed that the @a script data
     * remains available until parsing is completed (or the script is replaced).
     *
     * @param script      The script source to be parsed.
     * @param sourcePath  Used to identify the script in log messages. A copy is made.
     */
    void parse(Str const *script, Str const *sourcePath);

    bool readToken();
    Str const *token();

    void unreadToken();

    Str const *readString();
    int readNumber();

    AutoStr *readLumpName();
    int readSoundId();
    int readSoundIndex();
    uint readMapNumber();
    Uri *readTextureUri(char const *defaultScheme);

    int lineNumber() const;

    void scriptError();
    void scriptError(char const *message);

private:
    void checkOpen();
    bool atEnd();

    Str _sourcePath;    ///< Used to identify the source in error messages.

    Str const *_script; ///< The start of the script being parsed.
    int _readPos;       ///< Current read position.
    int _lineNumber;

    Str _token;
    int _tokenAsNumber;
    bool _alreadyGot;
    bool _multiline;    ///< @c true= current token spans multiple lines.
};

#endif // LIBCOMMON_HEXLEX_H
