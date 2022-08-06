/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/compiler.h>
#include <linux/export.h>
#include <linux/stats.h>
#include <linux/math.h>


inline struct mean_and_variance stats_update(struct mean_and_variance stat1, s64 val)
{
	struct mean_and_variance stat2;
	stat2.n           = stat1.n + 1;
	stat2.sum         = stat1.sum + val;
	stat2.sum_squares = stat1.sum_squares + val*val;
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
	s64 s1 = stat.sum_squares / stat.n;
	s64 s2 = stats_mean(stat);
	return s1 - s2*s2;
}
EXPORT_SYMBOL_GPL(stats_variance);

inline u64 stats_stddev(struct mean_and_variance stat)
{
	return int_sqrt64(stats_variance(stat));
}
EXPORT_SYMBOL_GPL(stats_stddev);

inline struct mean_and_variance_ewm stats_ewm_update(struct mean_and_variance_ewm sewm, s64 val)
{
	struct mean_and_variance_ewm sewm1;
	s64 m = sewm.mean;
	s64 v = sewm.variance;
	s64 w = sewm1.w = sewm.w;
	s64 val1 = val << w;
	s64 d = (val1 - m);
	s64 d1 = d >> w;
	s64 d2 = (d*d) >> w;

	if (!sewm.init) {
		sewm1.mean = val1;
		sewm1.variance = 0;
	} else {
		sewm1.mean = m + d1;
		sewm1.variance = v + ((d2 - (d2 >> w) - v) >> w);
	}
	sewm1.init = true;
	return sewm1;
}
EXPORT_SYMBOL_GPL(stats_ewm_update);

inline s64 stats_ewm_mean(struct mean_and_variance_ewm stat)
{
	return stat.mean >> stat.w;
}
EXPORT_SYMBOL_GPL(stats_ewm_mean);

inline u64 stats_ewm_variance(struct mean_and_variance_ewm stat)
{
	return stat.variance >> stat.w;
}
EXPORT_SYMBOL_GPL(stats_ewm_variance);

inline u64 stats_ewm_stddev(struct mean_and_variance_ewm stat)
{
	return int_sqrt64(stats_ewm_variance(stat));
}
EXPORT_SYMBOL_GPL(stats_ewm_stddev);
