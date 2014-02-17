/* Copyright (c) 2012-2013 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "build/Protocol/Raft.pb.h"
#include "Core/Debug.h"
#include "Core/ProtoBuf.h"
#include "Core/StringUtil.h"
#include "RPC/Buffer.h"
#include "RPC/ProtoBuf.h"
#include "Server/RaftLog.h"

namespace LogCabin {
namespace Server {
namespace RaftConsensusInternal {

////////// Log //////////

Log::Log()
    : metadata()
{
}

Log::~Log()
{
}

std::unique_ptr<Log::Sync>
Log::appendSingle(const Entry& entry)
{
    return append({&entry});
}

std::ostream&
operator<<(std::ostream& os, const Log& log)
{
    os << "Log:" << std::endl;
    os << "metadata: " << Core::ProtoBuf::dumpString(log.metadata);
    os << "startIndex: " << log.getLogStartIndex() << std::endl;
    for (uint64_t i = log.getLogStartIndex();
         i <= log.getLastLogIndex();
         ++i) {
        os << "Entry " << i << ": "
           << Core::ProtoBuf::dumpString(log.getEntry(i));
    }
    os << std::endl;
    return os;
}

} // namespace LogCabin::Server::RaftConsensusInternal
} // namespace LogCabin::Server
} // namespace LogCabin
