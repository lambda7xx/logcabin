/* Copyright (c) 2012-2014 Stanford University
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

#include <gtest/gtest.h>
#include <stdexcept>
#include "Server/MemoryLog.h"

namespace LogCabin {
namespace Server {
namespace {

using namespace RaftConsensusInternal; // NOLINT

// One thing to keep in mind for these tests is truncatePrefix. Calling that
// basically affects every other method, so every test should include
// a call to truncatePrefix.

class ServerMemoryLogTest : public ::testing::Test {
    ServerMemoryLogTest()
        : log()
        , sampleEntry()
    {
        sampleEntry.set_term(40);
        sampleEntry.set_data("foo");
    }
    MemoryLog log;
    MemoryLog::Entry sampleEntry;
};

TEST_F(ServerMemoryLogTest, basic)
{
    std::unique_ptr<Log::Sync> sync = log.appendSingle(sampleEntry);
    EXPECT_EQ(1U, sync->firstIndex);
    EXPECT_EQ(1U, sync->lastIndex);
    Log::Entry entry = log.getEntry(1);
    EXPECT_EQ(40U, entry.term());
    EXPECT_EQ("foo", entry.data());
}

TEST_F(ServerMemoryLogTest, append)
{
    std::unique_ptr<Log::Sync> sync = log.appendSingle(sampleEntry);
    EXPECT_EQ(1U, sync->firstIndex);
    EXPECT_EQ(1U, sync->lastIndex);
    log.truncatePrefix(10);
    sync = log.append({&sampleEntry, &sampleEntry});
    EXPECT_EQ(10U, sync->firstIndex);
    EXPECT_EQ(11U, sync->lastIndex);
    EXPECT_EQ(10U, log.getLogStartIndex());
    EXPECT_EQ(11U, log.getLastLogIndex());
}

TEST_F(ServerMemoryLogTest, getEntry)
{
    log.appendSingle(sampleEntry);
    Log::Entry entry = log.getEntry(1);
    EXPECT_EQ(40U, entry.term());
    EXPECT_EQ("foo", entry.data());
    EXPECT_THROW(log.getEntry(0), std::out_of_range);
    EXPECT_THROW(log.getEntry(2), std::out_of_range);

    sampleEntry.set_data("bar");
    log.appendSingle(sampleEntry);
    log.truncatePrefix(2);
    EXPECT_THROW(log.getEntry(1), std::out_of_range);
    log.appendSingle(sampleEntry);
    Log::Entry entry2 = log.getEntry(2);
    EXPECT_EQ("bar", entry2.data());
}

TEST_F(ServerMemoryLogTest, getLogStartIndex)
{
    EXPECT_EQ(1U, log.getLogStartIndex());
    log.truncatePrefix(200);
    log.truncatePrefix(100);
    EXPECT_EQ(200U, log.getLogStartIndex());
}

TEST_F(ServerMemoryLogTest, getLastLogIndex)
{
    EXPECT_EQ(0U, log.getLastLogIndex());
    log.appendSingle(sampleEntry);
    log.appendSingle(sampleEntry);
    EXPECT_EQ(2U, log.getLastLogIndex());

    log.truncatePrefix(2);
    EXPECT_EQ(2U, log.getLastLogIndex());
}

TEST_F(ServerMemoryLogTest, getSizeBytes)
{
    EXPECT_EQ(0U, log.getSizeBytes());
    log.appendSingle(sampleEntry);
    uint64_t s = log.getSizeBytes();
    EXPECT_LT(0U, s);
    log.appendSingle(sampleEntry);
    EXPECT_EQ(2 * s, log.getSizeBytes());
}

TEST_F(ServerMemoryLogTest, truncatePrefix)
{
    EXPECT_EQ(1U, log.startIndex);
    log.truncatePrefix(0);
    EXPECT_EQ(1U, log.startIndex);
    log.truncatePrefix(1);
    EXPECT_EQ(1U, log.startIndex);

    // case 1: entries is empty
    log.truncatePrefix(500);
    EXPECT_EQ(500U, log.startIndex);
    EXPECT_EQ(0U, log.entries.size());

    // case 2: entries has fewer elements than truncated
    log.appendSingle(sampleEntry);
    log.truncatePrefix(502);
    EXPECT_EQ(502U, log.startIndex);
    EXPECT_EQ(0U, log.entries.size());

    // case 3: entries has exactly the elements truncated
    log.appendSingle(sampleEntry);
    log.appendSingle(sampleEntry);
    log.truncatePrefix(504);
    EXPECT_EQ(504U, log.startIndex);
    EXPECT_EQ(0U, log.entries.size());

    // case 4: entries has more elements than truncated
    log.appendSingle(sampleEntry);
    log.appendSingle(sampleEntry);
    sampleEntry.set_data("bar");
    log.appendSingle(sampleEntry);
    log.truncatePrefix(506);
    EXPECT_EQ(506U, log.startIndex);
    EXPECT_EQ(1U, log.entries.size());
    EXPECT_EQ("bar", log.entries.at(0).data());

    // make sure truncating to an earlier id has no effect
    EXPECT_EQ(1U, log.entries.size());
    log.truncatePrefix(400);
    EXPECT_EQ(506U, log.startIndex);
}

TEST_F(ServerMemoryLogTest, truncateSuffix)
{
    log.truncateSuffix(0);
    log.truncateSuffix(10);
    EXPECT_EQ(0U, log.getLastLogIndex());
    log.appendSingle(sampleEntry);
    log.appendSingle(sampleEntry);
    log.truncateSuffix(10);
    EXPECT_EQ(2U, log.getLastLogIndex());
    log.truncateSuffix(2);
    EXPECT_EQ(2U, log.getLastLogIndex());
    log.truncateSuffix(1);
    EXPECT_EQ(1U, log.getLastLogIndex());
    log.truncateSuffix(0);
    EXPECT_EQ(0U, log.getLastLogIndex());


    log.truncatePrefix(10);
    log.appendSingle(sampleEntry);
    EXPECT_EQ(10U, log.getLastLogIndex());
    log.truncateSuffix(10);
    EXPECT_EQ(10U, log.getLastLogIndex());
    log.truncateSuffix(8);
    EXPECT_EQ(9U, log.getLastLogIndex());
    log.appendSingle(sampleEntry);
    EXPECT_EQ(10U, log.getLastLogIndex());
}

} // namespace LogCabin::Server::<anonymous>
} // namespace LogCabin::Server
} // namespace LogCabin
