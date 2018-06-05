#ifndef RTL_SENDFILE_H
#define RTL_SENDFILE_H

/*
 * G2 fast path support
 * Samba daemon passes flags to splice indicating if data is sent over G2 fast path.
 */
extern unsigned int rtl_use_sendfile;

struct rtl_sendfile_ops {
	ssize_t (*splice) (int fd_in, loff_t *off_out, int fd_out, size_t len, int *run_normal);
	int (*splice_from_pipe_feed) (struct pipe_inode_info *pipe, struct splice_desc *sd,
					splice_actor *actor);
	int (*pipe_to_sendpages) (struct pipe_inode_info *pipe,
					struct pipe_buffer *buf, struct splice_desc *sd);
	void (*tcp_ctl_init) (void);
	int (*skb_copy_datagram_to_kernel_iovec) (const struct sk_buff *skb, int offset,
					struct iovec *to, int len);
};

extern struct rtl_sendfile_ops *rtl_sendfile_hook;

#endif /* RTL_SENDFILE_H */
