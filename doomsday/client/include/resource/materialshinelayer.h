/** @file materialshinelayer.h  Logical material, shine/reflection layer.
 *
 * @authors Copyright © 2011-2014 Daniel Swanson <danij@dengine.net>
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

#ifndef CLIENT_RESOURCE_MATERIALSHINELAYER_H
#define CLIENT_RESOURCE_MATERIALSHINELAYER_H

#include <de/String>
#include <doomsday/defs/dedtypes.h>
#include "Material"
#include "Texture"

/**
 * Specialized Material::Layer for a describing an animated shine/reflection layer.
 *
 * @ingroup resource
 */
class MaterialShineLayer : public Material::Layer
{
public:
    /**
     * Stages describe texture change animations.
     */
    struct AnimationStage : public Stage
    {
    public:
        float shininess;
        blendmode_t blendMode;
        de::Vector3f minColor;
        de::Texture *maskTexture;
        de::Vector2f maskDimensions;

    public:
        AnimationStage(de::Texture *texture, int tics, float variance,
                       de::Texture *maskTexture           = 0,
                       blendmode_t blendMode              = BM_ADD,
                       float shininess                    = 1,
                       de::Vector3f const &minColor       = de::Vector3f(0, 0, 0),
                       de::Vector2f const &maskDimensions = de::Vector2f(1, 1));
        AnimationStage(AnimationStage const &other);
        virtual ~AnimationStage() {}

        /**
         * Construct a new AnimationStage from the given @a definition.
         */
        static AnimationStage *fromDef(ded_shine_stage_t const &definition);

        /**
         * Returns the Texture in effect for the animation stage.
         */
        de::Texture *texture() const;

        de::String description() const;

    private:
        de::Texture *_texture;  ///< Not owned.
    };

public:
    virtual ~MaterialShineLayer() {}

    de::String describe() const;

    /**
     * Construct a new layer from the specified definition.
     */
    static MaterialShineLayer *fromDef(ded_reflection_t const &def);

    /**
     * Add a new animation Stage to the layer.
     *
     * @param stage  New Stage to add (a copy is made).
     *
     * @return  Index of the newly added stage (0 based).
     */
    int addStage(AnimationStage const &stage);

    /**
     * Lookup an AnimationStage by it's unique @a index.
     *
     * @param index  Index of the AnimationStage to lookup. Will be cycled into valid range.
     */
    AnimationStage &stage(int index) const;
};

#endif  // CLIENT_RESOURCE_MATERIALSHINELAYER_H