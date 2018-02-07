/** @file glpixelformat.h Pixel format specification for GL.
 *
 * @authors Copyright (c) 2013-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#ifndef LIBGUI_GLPIXELFORMAT_H
#define LIBGUI_GLPIXELFORMAT_H

#include "../gui/libgui.h"

namespace de {

/**
 * GL image format with data type for glTex(Sub)Image.
 *
 * @ingroup gl
 */
struct LIBGUI_PUBLIC GLPixelFormat {
    duint internalFormat;
    duint format;
    duint type;
    duint rowAlignment;

    GLPixelFormat(duint glInternalFormat, duint glFormat, duint glDataType = 0,
                  duint glRowAlignment = 0)
        : internalFormat(glInternalFormat)
        , format(glFormat)
        , type(glDataType)
        , rowAlignment(glRowAlignment)
    {}
};

} // namespace de

#endif // LIBGUI_GLPIXELFORMAT_H
