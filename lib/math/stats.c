/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/compiler.h>
#include <linux/printbuf.h>
#include <linux/export.h>
#include <linux/limits.h>
#include <linux/math64.h>
#include <linux/stats.h>
#include <linux/math.h>
#include <linux/bug.h>

#define FAST_DIVPOW2(n, d) (((n) >> (d)) + (((n) < 0 && ((-(n) >> ((d)-1)) & 1)) ? 1 : 0))

inline s64 fast_divpow2(s64 n, u8 d)
{
	return (n + ((n < 0) ? ((1 << d) - 1) : 0)) >> d; // + (n < 0 ? 1 : 0);
}

inline struct mean_and_variance stats_update(struct mean_and_variance stat1, s64 val)
{
	struct mean_and_variance stat2;
	u64 val2 = abs(val);

	WARN(val2 > SQRT_U64_MAX, "stats overflow! %lld^2 > U64_MAX", val);

	stat2.n           = stat1.n + 1;
	stat2.sum         = stat1.sum + val;
	stat2.sum_squares = stat1.sum_squares + val2*val2;
	return stat2;
}
EXPORT_SYMBOL_GPL(stats_update);

inline s64 stats_mean(struct mean_and_variance stat)
{
	return stat.sum / stat.n;
}
EXPORT_SYMBOL_GPL(stats_mean);

inline u64 stats_variance(struct mean_and_variance stat)
{
	u64 s1 = stat.sum_squares / stat.n;
	u64 s2 = abs(stats_mean(stat));
	WARN(s2 > SQRT_U64_MAX, "stats overflow %lld ^2 > S64_MAX", s2);
	return s1 - s2*s2;
}
EXPORT_SYMBOL_GPL(stats_variance);

inline u32 stats_stddev(struct mean_and_variance stat)
{
	return int_sqrt64(stats_variance(stat));
}
EXPORT_SYMBOL_GPL(stats_stddev);

inline struct mean_and_variance_ewm stats_ewm_update(struct mean_and_variance_ewm sewm, s64 val)
{
	struct mean_and_variance_ewm sewm1;
	s64 m = sewm.mean;
	u64 v = sewm.variance;
	u8 w = sewm1.w = sewm.w;
	s64 val1 = val << w;
	s64 d = (val1 - m);
	s64 d1 = fast_divpow2(d, w);
	u64 d2 = (d*d) >> w;

	if (!sewm.init) {
		sewm1.mean = val1;
		sewm1.variance = 0;
	} else {
		sewm1.mean = m + d1;
		sewm1.variance = v + ((d2 - (d2 >> w) - v) >> w);
	}
	sewm1.init = true;

	#ifdef CONFIG_STATS_UNIT_TEST
	printk("val = %lld, val1 = %lld, d = %lld, d1 = %lld, d2 = %llu, mean = %lld, vari = %llu",
		   val, val1, d, d1, d2, sewm1.mean, sewm1.variance);
	#endif
	return sewm1;
}
EXPORT_SYMBOL_GPL(stats_ewm_update);

inline s64 stats_ewm_mean(struct mean_and_variance_ewm stat)
{
	return fast_divpow2(stat.mean, stat.w);
}
EXPORT_SYMBOL_GPL(stats_ewm_mean);

inline u64 stats_ewm_variance(struct mean_and_variance_ewm stat)
{
	// always positive don't need fast divpow2
	return stat.variance >> stat.w;
}
EXPORT_SYMBOL_GPL(stats_ewm_variance);

inline u32 stats_ewm_stddev(struct mean_and_variance_ewm stat)
{
	return int_sqrt64(stats_ewm_variance(stat));
}
EXPORT_SYMBOL_GPL(stats_ewm_stddev);
