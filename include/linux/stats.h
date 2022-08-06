#ifndef STATS_H_
#define STATS_H_

#include <linux/types.h>


struct mean_and_variance {
	s64 n;
	s64 sum;
	u64 sum_squares;
};

/* expontentially weighted moving average & variance */
struct mean_and_variance_ewm {
	bool init;
	s64 w;
	s64 mean;
	u64 variance;
};

struct mean_and_variance stats_update(struct mean_and_variance, s64);
       s64             stats_mean(struct mean_and_variance);
       u64             stats_variance(struct mean_and_variance);
       u64             stats_stddev(struct mean_and_variance);

struct mean_and_variance_ewm stats_ewm_update(struct mean_and_variance_ewm, s64);
       s64             stats_ewm_mean(struct mean_and_variance_ewm);
       u64             stats_ewm_variance(struct mean_and_variance_ewm);
       u64             stats_ewm_stddev(struct mean_and_variance_ewm);

#endif // STATS_H_
