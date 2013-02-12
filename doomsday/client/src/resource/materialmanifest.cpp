/** @file materialmanifest.cpp Description of a logical material resource.
 *
 * @authors Copyright � 2011-2013 Daniel Swanson <danij@dengine.net>
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

#include "de_base.h"

#include "Materials"
#include "MaterialManifest"

namespace de {

DENG2_PIMPL(MaterialManifest)
{
    /// Material associated with this.
    Material *material;

    /// Unique identifier.
    materialid_t id;

    /// Classification flags.
    MaterialManifest::Flags flags;

    Instance(Public &a) : Base(a),
        material(0),
        id(0)
    {}
};

MaterialManifest::MaterialManifest(PathTree::NodeArgs const &args)
    : Node(args), d(new Instance(*this))
{}

MaterialManifest::~MaterialManifest()
{
    delete d;
}

Materials &MaterialManifest::materials()
{
    return App_Materials();
}

materialid_t MaterialManifest::id() const
{
    return d->id;
}

void MaterialManifest::setId(materialid_t id)
{
    d->id = id;
}

MaterialScheme &MaterialManifest::scheme() const
{
    LOG_AS("MaterialManifest::scheme");
    /// @todo Optimize: MaterialManifest should contain a link to the owning MaterialScheme.
    foreach(Materials::Scheme *scheme, materials().allSchemes())
    {
        if(&scheme->index() == &tree()) return *scheme;
    }
    /// @throw Error Failed to determine the scheme of the manifest (should never happen...).
    throw Error("MaterialManifest::scheme", String("Failed to determine scheme for manifest [%1]").arg(de::dintptr(this)));
}

Uri MaterialManifest::composeUri(QChar sep) const
{
    return Uri(schemeName(), path(sep));
}

String MaterialManifest::sourceDescription() const
{
    if(!isCustom()) return "game";
    if(isAutoGenerated()) return "add-on"; // Unintuitive but correct.
    return "def";
}

MaterialManifest::Flags MaterialManifest::flags() const
{
    return d->flags;
}

void MaterialManifest::setFlags(MaterialManifest::Flags flagsToChange, bool set)
{
    if(set)
    {
        d->flags |= flagsToChange;
    }
    else
    {
        d->flags &= ~flagsToChange;
    }
}

bool MaterialManifest::hasMaterial() const
{
    return !!d->material;
}

Material &MaterialManifest::material() const
{
    if(!d->material)
    {
        /// @throw MissingMaterialError  The manifest is not presently associated with a material.
        throw MissingMaterialError("MaterialManifest::material", "Missing required material");
    }
    return *d->material;
}

void MaterialManifest::setMaterial(Material *newMaterial)
{
    if(d->material == newMaterial) return;
    d->material = newMaterial;
}

} // namespace de
