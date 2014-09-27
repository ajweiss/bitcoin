// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for blockinv queue 
//

#include "main.h"
#include "util.h"

#include <stdint.h>

#include <boost/test/unit_test.hpp>

// Tests this internal-to-main.cpp method:

using namespace std;

extern map<uint256, list<NodeId> > mapBlockInvQueue;
extern map<NodeId, map<uint256, list<NodeId>::iterator> > mapBlockInvQueueByNode;

extern size_t BlockInvAddToQueue(const uint256 &hash, NodeId nodeid);
extern void BlockInvClearFromQueue(const uint256 &hash);
extern void BlockInvForgetNode(NodeId nodeid);
extern NodeId BlockInvDequeueBestNode(const uint256 &hash);

struct BlockInvTestSetup {
    BlockInvTestSetup() {
        mapBlockInvQueue.clear();
        mapBlockInvQueueByNode.clear();
    }
};

BOOST_FIXTURE_TEST_SUITE(blockinv_tests, BlockInvTestSetup)

BOOST_AUTO_TEST_CASE(bi_create_and_dequeue)
{
    uint256 hash = 100;
    NodeId nid = 200;
    BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash, nid), 1);
    BOOST_CHECK_EQUAL(BlockInvDequeueBestNode(hash), nid);
    BOOST_CHECK_EQUAL(mapBlockInvQueue.count(hash), 0);
    BOOST_CHECK_EQUAL(mapBlockInvQueueByNode.count(nid), 1);
    BOOST_CHECK_EQUAL(mapBlockInvQueueByNode[nid].size(), 0);
}

BOOST_AUTO_TEST_CASE(bi_create_and_forget_node)
{
    uint256 hash = 300;
    NodeId nid = 400;
    BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash, nid), 1);
    BlockInvForgetNode(nid);
    BOOST_CHECK_EQUAL(mapBlockInvQueue.count(hash), 0);
    BOOST_CHECK_EQUAL(mapBlockInvQueueByNode.count(nid), 0);
}

BOOST_AUTO_TEST_CASE(bi_clear_inv)
{
    uint256 hash = 300;
    NodeId nid = 400;
    BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash, nid), 1);
    BlockInvClearFromQueue(hash);
    BOOST_CHECK_EQUAL(mapBlockInvQueue.count(hash), 0);
    BOOST_CHECK_EQUAL(mapBlockInvQueueByNode.count(nid), 1);
}

BOOST_AUTO_TEST_CASE(bi_queue_order)
{
    uint256 hash = 500;
    NodeId nid = 600;
    BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash, nid), 1);
    BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash, nid+1), 1);
    BOOST_CHECK_EQUAL(BlockInvDequeueBestNode(hash), nid);
    BOOST_CHECK_EQUAL(BlockInvDequeueBestNode(hash), nid+1);
}

BOOST_AUTO_TEST_CASE(bi_lots_of_data)
{
    uint256 hash = 10000;
    // add 20 nodes with 5000 invs each
    for (int i = 0; i < 5000; i++) {
        for (int j = 0; j < 20; j++) {
            BOOST_CHECK_EQUAL(BlockInvAddToQueue(hash+i, (i+j)%20), i+1);
        }
    }

    // forget the even nodes
    for (int i = 0; i < 20; i+=2)
        BlockInvForgetNode(i);

    // clear the even hashes 
    for (int i = 0; i < 5000; i+=2)
        BlockInvClearFromQueue(hash+i);
  
    // dequeue the odd nodes from the odd hashes 
    for (int i = 1; i < 5000; i+=2) {
        for (int j = 0; j < 20; j+=2) {
            BOOST_CHECK_EQUAL(BlockInvDequeueBestNode(hash+i), (i+j)%20);
        }
    }

    // verify no hashes left
    BOOST_CHECK_EQUAL(mapBlockInvQueue.count(hash), 0);

    // verify odd nodes left
    for (int i = 1; i < 20; i+=2) {
        BOOST_CHECK_EQUAL(mapBlockInvQueueByNode.count(i), 1);
        BOOST_CHECK_EQUAL(mapBlockInvQueueByNode[i].size(), 0);
    }

    // now forget them and verify everything is empty
    for (int i = 1; i < 20; i+=2)
        BlockInvForgetNode(i);

    BOOST_CHECK_EQUAL(mapBlockInvQueueByNode.size(), 0);
    BOOST_CHECK_EQUAL(mapBlockInvQueue.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
