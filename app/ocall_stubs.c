#include <sgx_edger8r.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

// 基本的OCall函数存根实现
void u_getuid_ocall(void* retval) { *(int*)retval = 0; }
void u_getgid_ocall(void* retval) { *(int*)retval = 0; }
void u_getcwd_ocall(void* retval, char* buf, size_t size) { *(int*)retval = -1; }
void u_chdir_ocall(void* retval, const char* path) { *(int*)retval = -1; }
void u_free_ocall(void* ptr) { /* 空实现 */ }
void u_read_ocall(void* retval, int fd, void* buf, size_t count) { *(int*)retval = -1; }
void u_pread64_ocall(void* retval, int fd, void* buf, size_t count, long offset) { *(int*)retval = -1; }
void u_write_ocall(void* retval, int fd, const void* buf, size_t count) { *(int*)retval = -1; }
void u_pwrite64_ocall(void* retval, int fd, const void* buf, size_t count, long offset) { *(int*)retval = -1; }
void u_open_ocall(void* retval, const char* pathname, int flags) { *(int*)retval = -1; }
void u_close_ocall(void* retval, int fd) { *(int*)retval = -1; }
void u_fstat_ocall(void* retval, int fd, void* buf) { *(int*)retval = -1; }
void u_malloc_ocall(void* retval, size_t size) { *(void**)retval = NULL; }
void u_socket_ocall(void* retval, int domain, int type, int protocol) { *(int*)retval = -1; }
void u_connect_ocall(void* retval, int sockfd, const void* addr, unsigned int addrlen) { *(int*)retval = -1; }
void u_bind_ocall(void* retval, int sockfd, const void* addr, unsigned int addrlen) { *(int*)retval = -1; }
void u_listen_ocall(void* retval, int sockfd, int backlog) { *(int*)retval = -1; }
void u_accept4_ocall(void* retval, int sockfd, void* addr, unsigned int* addrlen, int flags) { *(int*)retval = -1; }
void u_send_ocall(void* retval, int sockfd, const void* buf, size_t len, int flags) { *(int*)retval = -1; }
void u_recv_ocall(void* retval, int sockfd, void* buf, size_t len, int flags) { *(int*)retval = -1; }
void u_sendto_ocall(void* retval, int sockfd, const void* buf, size_t len, int flags, const void* dest_addr, unsigned int addrlen) { *(int*)retval = -1; }
void u_recvfrom_ocall(void* retval, int sockfd, void* buf, size_t len, int flags, void* src_addr, unsigned int* addrlen) { *(int*)retval = -1; }
void u_setsockopt_ocall(void* retval, int sockfd, int level, int optname, const void* optval, unsigned int optlen) { *(int*)retval = -1; }
void u_getsockopt_ocall(void* retval, int sockfd, int level, int optname, void* optval, unsigned int* optlen) { *(int*)retval = -1; }
void u_getpeername_ocall(void* retval, int sockfd, void* addr, unsigned int* addrlen) { *(int*)retval = -1; }
void u_getsockname_ocall(void* retval, int sockfd, void* addr, unsigned int* addrlen) { *(int*)retval = -1; }
void u_shutdown_ocall(void* retval, int sockfd, int how) { *(int*)retval = -1; }
void u_stat_ocall(void* retval, const char* path, void* buf) { *(int*)retval = -1; }
void u_lstat_ocall(void* retval, const char* path, void* buf) { *(int*)retval = -1; }
void u_lseek_ocall(void* retval, int fd, long offset, int whence) { *(long*)retval = -1; }
void u_ftruncate_ocall(void* retval, int fd, long length) { *(int*)retval = -1; }
void u_fsync_ocall(void* retval, int fd) { *(int*)retval = -1; }
void u_fchmod_ocall(void* retval, int fd, unsigned int mode) { *(int*)retval = -1; }
void u_unlink_ocall(void* retval, const char* pathname) { *(int*)retval = -1; }
void u_link_ocall(void* retval, const char* oldpath, const char* newpath) { *(int*)retval = -1; }
void u_rename_ocall(void* retval, const char* oldpath, const char* newpath) { *(int*)retval = -1; }
void u_chmod_ocall(void* retval, const char* path, unsigned int mode) { *(int*)retval = -1; }
void u_readlink_ocall(void* retval, const char* pathname, char* buf, size_t bufsiz) { *(int*)retval = -1; }
void u_symlink_ocall(void* retval, const char* target, const char* linkpath) { *(int*)retval = -1; }
void u_realpath_ocall(void* retval, const char* path, char* resolved_path) { *(char**)retval = NULL; }
void u_mkdir_ocall(void* retval, const char* pathname, unsigned int mode) { *(int*)retval = -1; }
void u_rmdir_ocall(void* retval, const char* pathname) { *(int*)retval = -1; }
void u_opendir_ocall(void* retval, const char* name) { *(void**)retval = NULL; }
void u_readdir64_r_ocall(void* retval, void* dirp, void* entry, void** result) { *(int*)retval = -1; }
void u_closedir_ocall(void* retval, void* dirp) { *(int*)retval = -1; }
void u_dirfd_ocall(void* retval, void* dirp) { *(int*)retval = -1; }
void u_poll_ocall(void* retval, void* fds, unsigned int nfds, int timeout) { *(int*)retval = -1; }
void u_epoll_create1_ocall(void* retval, int flags) { *(int*)retval = -1; }
void u_epoll_ctl_ocall(void* retval, int epfd, int op, int fd, void* event) { *(int*)retval = -1; }
void u_epoll_wait_ocall(void* retval, int epfd, void* events, int maxevents, int timeout) { *(int*)retval = -1; }
void u_pipe2_ocall(void* retval, int pipefd[2], int flags) { *(int*)retval = -1; }
void u_getpid_ocall(void* retval) { *(int*)retval = 0; }
void u_sysconf_ocall(void* retval, int name) { *(long*)retval = -1; }
void u_prctl_ocall(void* retval, int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) { *(int*)retval = -1; }
void u_sched_getaffinity_ocall(void* retval, int pid, size_t cpusetsize, void* cpuset) { *(int*)retval = -1; }
void u_sched_setaffinity_ocall(void* retval, int pid, size_t cpusetsize, const void* cpuset) { *(int*)retval = -1; }
void u_sched_yield_ocall(void* retval) { *(int*)retval = 0; }
void u_nanosleep_ocall(void* retval, const void* req, void* rem) { *(int*)retval = 0; }
void u_clock_gettime_ocall(void* retval, int clk_id, void* tp) { *(int*)retval = -1; }
void u_thread_wait_event_ocall(void* retval, void* tcs) { *(int*)retval = 0; }
void u_thread_set_multiple_events_ocall(void* retval, void* tcs, int count) { *(int*)retval = 0; }
void sgx_oc_cpuidex(void* retval, void* cpuinfo, int leaf, int subleaf) { /* 空实现 */ }
void u_fcntl_arg0_ocall(void* retval, int fd, int cmd) { *(int*)retval = -1; }
void u_fcntl_arg1_ocall(void* retval, int fd, int cmd, int arg) { *(int*)retval = -1; }
void u_ioctl_arg0_ocall(void* retval, int fd, unsigned long request) { *(int*)retval = -1; }
void u_ioctl_arg1_ocall(void* retval, int fd, unsigned long request, void* arg) { *(int*)retval = -1; }
void u_isatty_ocall(void* retval, int fd) { *(int*)retval = 0; }
void u_dup_ocall(void* retval, int oldfd) { *(int*)retval = -1; }
void u_eventfd_ocall(void* retval, unsigned int initval, int flags) { *(int*)retval = -1; }
void u_futimens_ocall(void* retval, int fd, const void* times) { *(int*)retval = -1; }
void u_sendfile_ocall(void* retval, int out_fd, int in_fd, void* offset, size_t count) { *(int*)retval = -1; }
void u_copy_file_range_ocall(void* retval, int fd_in, void* off_in, int fd_out, void* off_out, size_t len, unsigned int flags) { *(int*)retval = -1; }
void u_splice_ocall(void* retval, int fd_in, void* off_in, int fd_out, void* off_out, size_t len, unsigned int flags) { *(int*)retval = -1; }
void u_readv_ocall(void* retval, int fd, const void* iov, int iovcnt) { *(int*)retval = -1; }
void u_preadv64_ocall(void* retval, int fd, const void* iov, int iovcnt, long offset) { *(int*)retval = -1; }
void u_writev_ocall(void* retval, int fd, const void* iov, int iovcnt) { *(int*)retval = -1; }
void u_pwritev64_ocall(void* retval, int fd, const void* iov, int iovcnt, long offset) { *(int*)retval = -1; }
void u_open64_ocall(void* retval, const char* pathname, int flags) { *(int*)retval = -1; }
void u_openat_ocall(void* retval, int dirfd, const char* pathname, int flags) { *(int*)retval = -1; }
void u_fstat64_ocall(void* retval, int fd, void* buf) { *(int*)retval = -1; }
void u_stat64_ocall(void* retval, const char* path, void* buf) { *(int*)retval = -1; }
void u_lstat64_ocall(void* retval, const char* path, void* buf) { *(int*)retval = -1; }
void u_lseek64_ocall(void* retval, int fd, long offset, int whence) { *(long*)retval = -1; }
void u_ftruncate64_ocall(void* retval, int fd, long length) { *(int*)retval = -1; }
void u_truncate_ocall(void* retval, const char* path, long length) { *(int*)retval = -1; }
void u_truncate64_ocall(void* retval, const char* path, long length) { *(int*)retval = -1; }
void u_fdatasync_ocall(void* retval, int fd) { *(int*)retval = -1; }
void u_unlinkat_ocall(void* retval, int dirfd, const char* pathname, int flags) { *(int*)retval = -1; }
void u_linkat_ocall(void* retval, int olddirfd, const char* oldpath, int newdirfd, const char* newpath, int flags) { *(int*)retval = -1; }
void u_fdopendir_ocall(void* retval, int fd) { *(void**)retval = NULL; }
void u_fstatat64_ocall(void* retval, int dirfd, const char* pathname, void* statbuf, int flags) { *(int*)retval = -1; }
void u_getaddrinfo_ocall(void* retval, const char* node, const char* service, const void* hints, void** res) { *(int*)retval = -1; }
void u_socketpair_ocall(void* retval, int domain, int type, int protocol, int sv[2]) { *(int*)retval = -1; }