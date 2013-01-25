/** @file link.h  Network connection to a server.
 *
 * @authors Copyright © 2013 Jaakko Keränen <jaakko.keranen@iki.fi>
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
 * General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "de/shell/Link"
#include "de/shell/Protocol"
#include <de/Message>
#include <de/Socket>
#include <de/Time>
#include <de/Log>

namespace de {
namespace shell {

struct Link::Instance
{
    Link &self;
    Address serverAddress;
    Socket socket;
    Protocol protocol;
    Status status;
    Time connectedAt;

    Instance(Link &i)
        : self(i),
          status(Disconnected),
          connectedAt(Time::invalidTime()) {}

    ~Instance() {}
};

Link::Link(Address const &address) : d(new Instance(*this))
{
    d->serverAddress = address;

    connect(&d->socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(&d->socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(&d->socket, SIGNAL(messagesReady()), this, SIGNAL(packetsReady()));

    d->socket.connect(address);

    d->status = Connecting;
}

Link::~Link()
{
    delete d;
}

Address Link::address() const
{
    if(d->socket.isOpen()) return d->socket.peerAddress();
    return d->serverAddress;
}

Link::Status Link::status() const
{
    return d->status;
}

Time Link::connectedAt() const
{
    return d->connectedAt;
}

Packet *Link::nextPacket()
{
    if(!d->socket.hasIncoming()) return 0;

    std::auto_ptr<Message> data(d->socket.receive());
    return d->protocol.interpret(*data.get());
}

void Link::send(IByteArray const &data)
{
    d->socket.send(data);
}

void Link::socketConnected()
{
    LOG_AS("Link");
    LOG_VERBOSE("Successfully connected to %s") << d->socket.peerAddress();

    d->status = Connected;
    d->connectedAt = Time();

    emit connected();
}

void Link::socketDisconnected()
{
    LOG_AS("Link");
    LOG_INFO("Disconnected from %s") << d->serverAddress;

    d->status = Disconnected;

    emit disconnected();

    // Slots have now had an opportunity to observe the total
    // duration of the connection that has just ended.
    d->connectedAt = Time::invalidTime();
}

} // namespace shell
} // namespace de
