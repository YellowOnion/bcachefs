/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BCACHEFS_FS_IO_H
#define _BCACHEFS_FS_IO_H

#ifndef NO_BCACHEFS_FS

#include "buckets.h"
#include "fs.h"
#include "io_write_types.h"
#include "quota.h"

#include <linux/uio.h>

struct quota_res {
	u64				sectors;
};

#ifdef CONFIG_BCACHEFS_QUOTA

static inline void __bch2_quota_reservation_put(struct bch_fs *c,
					 struct bch_inode_info *inode,
					 struct quota_res *res)
{
	BUG_ON(res->sectors > inode->ei_quota_reserved);

	bch2_quota_acct(c, inode->ei_qid, Q_SPC,
			-((s64) res->sectors), KEY_TYPE_QUOTA_PREALLOC);
	inode->ei_quota_reserved -= res->sectors;
	res->sectors = 0;
}

static inline void bch2_quota_reservation_put(struct bch_fs *c,
				       struct bch_inode_info *inode,
				       struct quota_res *res)
{
	if (res->sectors) {
		mutex_lock(&inode->ei_quota_lock);
		__bch2_quota_reservation_put(c, inode, res);
		mutex_unlock(&inode->ei_quota_lock);
	}
}

static inline int bch2_quota_reservation_add(struct bch_fs *c,
				      struct bch_inode_info *inode,
				      struct quota_res *res,
				      u64 sectors,
				      bool check_enospc)
{
	int ret;

	if (test_bit(EI_INODE_SNAPSHOT, &inode->ei_flags))
		return 0;

	mutex_lock(&inode->ei_quota_lock);
	ret = bch2_quota_acct(c, inode->ei_qid, Q_SPC, sectors,
			      check_enospc ? KEY_TYPE_QUOTA_PREALLOC : KEY_TYPE_QUOTA_NOCHECK);
	if (likely(!ret)) {
		inode->ei_quota_reserved += sectors;
		res->sectors += sectors;
	}
	mutex_unlock(&inode->ei_quota_lock);

	return ret;
}

#else

static inline void __bch2_quota_reservation_put(struct bch_fs *c,
					 struct bch_inode_info *inode,
					 struct quota_res *res) {}

static inline void bch2_quota_reservation_put(struct bch_fs *c,
				       struct bch_inode_info *inode,
				       struct quota_res *res) {}

static inline int bch2_quota_reservation_add(struct bch_fs *c,
				      struct bch_inode_info *inode,
				      struct quota_res *res,
				      unsigned sectors,
				      bool check_enospc)
{
	return 0;
}

#endif

void __bch2_i_sectors_acct(struct bch_fs *, struct bch_inode_info *,
			   struct quota_res *, s64);

static inline void bch2_i_sectors_acct(struct bch_fs *c, struct bch_inode_info *inode,
				       struct quota_res *quota_res, s64 sectors)
{
	if (sectors) {
		mutex_lock(&inode->ei_quota_lock);
		__bch2_i_sectors_acct(c, inode, quota_res, sectors);
		mutex_unlock(&inode->ei_quota_lock);
	}
}

static inline struct address_space *faults_disabled_mapping(void)
{
	return (void *) (((unsigned long) current->faults_disabled_mapping) & ~1UL);
}

static inline void set_fdm_dropped_locks(void)
{
	current->faults_disabled_mapping =
		(void *) (((unsigned long) current->faults_disabled_mapping)|1);
}

static inline bool fdm_dropped_locks(void)
{
	return ((unsigned long) current->faults_disabled_mapping) & 1;
}

void bch2_inode_flush_nocow_writes_async(struct bch_fs *,
			struct bch_inode_info *, struct closure *);

int __must_check bch2_write_inode_size(struct bch_fs *,
				       struct bch_inode_info *,
				       loff_t, unsigned);

int bch2_fsync(struct file *, loff_t, loff_t, int);

int bchfs_truncate(struct mnt_idmap *,
		  struct bch_inode_info *, struct iattr *);
long bch2_fallocate_dispatch(struct file *, int, loff_t, loff_t);

loff_t bch2_remap_file_range(struct file *, loff_t, struct file *,
			     loff_t, loff_t, unsigned);

loff_t bch2_llseek(struct file *, loff_t, int);

void bch2_fs_fsio_exit(struct bch_fs *);
int bch2_fs_fsio_init(struct bch_fs *);
#else
static inline void bch2_fs_fsio_exit(struct bch_fs *c) {}
static inline int bch2_fs_fsio_init(struct bch_fs *c) { return 0; }
#endif

#endif /* _BCACHEFS_FS_IO_H */
