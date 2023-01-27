#include <gtest/gtest.h>
#include <exception>
#include "globalincs/pool.h"
#include "tl/optional.hpp"
struct testobj{
	int id;
};

TEST(Pool, safe_access){
	SCP_Pool<testobj> store;
	auto a = store.get_new().value();
	auto b = store.get_new().value();
	store[a].id=1;
	store[b].id=2;
	store.remove(b);
	ASSERT_EQ(store[a].id, 1);
}

TEST(Pool, bad_access){
	SCP_Pool<testobj> store;
	auto a = store.get_new().value();
	auto b = store.get_new().value();
	store[a].id=1;
	store[b].id=2;
	store.remove(b);
	ASSERT_THROW(store[b], PoolExceptionStaleIndex);
}

TEST(Pool, safe_reuse){
	SCP_Pool<testobj> store;
	auto a = store.get_new().value();
	auto b = store.get_new().value();
	store[a].id=1;
	store[b].id=2;
	store.remove(a);
	auto c = store.get_new().value();
	auto *cptr = &store[c];
	cptr->id=3;
	ASSERT_EQ(a.i(),c.i());
	ASSERT_NE(a.g(),c.g());
	ASSERT_THROW(store[a], PoolExceptionStaleIndex);
	ASSERT_EQ(store[c].id, 3);
}


TEST(Pool, minimal){
	SCP_Pool<testobj> store;
	auto a = store.get_new().value();
	auto b = store.get_new().value();
	store[a].id=1;
	auto *bptr = &store[b];
	bptr->id=3;
	ASSERT_EQ(store[b].id, 3);
}


TEST(Pool, uid){
	SCP_Pool<testobj> store1;
	SCP_Pool<testobj> store2;
	auto a = store1.get_new().value();
	auto b = store2.get_new().value();
	store1[a].id;
	store2[b].id;
	ASSERT_THROW(store1[b], PoolExceptionWrongPool);
}
