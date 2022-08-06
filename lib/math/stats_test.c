/* SPDX-License-Identifier: GPL-2.0 */
#include <kunit/test.h>
#include <linux/stats.h>

static void stats_basic_test(struct kunit *test)
{
	struct mean_and_variance s = {};
	s = stats_update(s, 2);
	s = stats_update(s, 2);
	KUNIT_EXPECT_EQ(test, stats_mean(s), 2);
	KUNIT_EXPECT_EQ(test, stats_variance(s), 0);
	KUNIT_EXPECT_EQ(test, s.n, 2);

	s = stats_update(s, 4);
	s = stats_update(s, 4);

	KUNIT_EXPECT_EQ(test, stats_mean(s), 3);
	KUNIT_EXPECT_EQ(test, stats_variance(s), 1);
	KUNIT_EXPECT_EQ(test, s.n, 4);
}

static void stats_ewm_test(struct kunit *test)
{
	struct mean_and_variance_ewm s = {};

	s.w = 2;

	s = stats_ewm_update(s, 10);
	KUNIT_EXPECT_EQ(test, stats_ewm_mean(s), 10);
	KUNIT_EXPECT_EQ(test, stats_ewm_variance(s), 0);
	s = stats_ewm_update(s, 20);
	KUNIT_EXPECT_EQ(test, stats_ewm_mean(s), 12);
	KUNIT_EXPECT_EQ(test, stats_ewm_variance(s), 18);
	s = stats_ewm_update(s, 30);

	KUNIT_EXPECT_EQ(test, stats_ewm_mean(s), 16);
	KUNIT_EXPECT_EQ(test, stats_ewm_variance(s), 71);
}

static void stats_ewm_advanced_test(struct kunit *test)
{
	struct mean_and_variance_ewm s = {};
	u64 i;

	s.w = 8;

	for (i = 10; i <= 100; i+=10)
		s = stats_ewm_update(s, i);

	KUNIT_EXPECT_EQ(test, stats_ewm_mean(s), 11);
	KUNIT_EXPECT_EQ(test, stats_ewm_variance(s), 107);
}

static struct kunit_case stats_test_cases[] = {
	KUNIT_CASE(stats_basic_test),
	KUNIT_CASE(stats_ewm_test),
	KUNIT_CASE(stats_ewm_advanced_test),
	{}
};

static struct kunit_suite stats_test_suite = {
.name = "statistics",
.test_cases = stats_test_cases
};

kunit_test_suite(stats_test_suite);
