/** @file user.h
 *
 * @authors Copyright (c) 2018 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#ifndef GLOOM_USER_H
#define GLOOM_USER_H

#include <de/Vector>
#include <de/Time>
#include <de/Observers>
#include <de/Sound>
#include "../libgloom.h"

namespace gloom {

using namespace de;

class World;

class LIBGLOOM_PUBLIC User
{
public:
    enum InputBit {
        Inert     = 0,
        Shift     = 0x1,
        TurnLeft  = 0x2,
        TurnRight = 0x4,
        Forward   = 0x8,
        Backward  = 0x10,
        StepLeft  = 0x20,
        StepRight = 0x40,
        Jump      = 0x80,
    };
    using InputState = Flags;

    DE_DEFINE_AUDIENCE(Deletion,  void userBeingDeleted  (User &))
    DE_DEFINE_AUDIENCE(Warp,      void userWarped        (User const &))
    DE_DEFINE_AUDIENCE(PainLevel, void userPainLevel     (User const &, float pain))
    DE_DEFINE_AUDIENCE(Move,      void userMoved         (User const &, Vec3f const &pos))
    DE_DEFINE_AUDIENCE(Turn,      void userTurned        (User const &, float yaw))

public:
    User();

    void setWorld(World const *world);
    void setPosition(Vec3f const &pos);
    void setYaw(float yaw);
    void setPain(float pain);
    void setInputState(InputState const &state);
    void turn(float yaw, float pitch);
    void turn(const Vec2f &angles) { turn(angles.x, angles.y); }

    void update(TimeSpan elapsed);

    Vec3f  position() const; // User eye position.
    float  yaw() const;
    float  pitch() const;
    Sound &fastWindSound();

private:
    DE_PRIVATE(d)
};

} // namespace gloom

#endif // GLOOM_USER_H
