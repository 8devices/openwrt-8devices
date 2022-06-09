#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "cyg_crc.h"
#include "sha1.h"

#define PATH_LEN 1024
#define VERSION_LEN 128
struct cli_options {
	char kernel[PATH_LEN];
	char rootfs[PATH_LEN];
	char version[VERSION_LEN];
};

#define SHA1_LEN 20
#define INFO_IMG_MAG 0xFEEDFACE
#define INFO_IMG_RV_LEN 128
struct info_img {
	uint32_t magic;
	uint8_t rev[INFO_IMG_RV_LEN];
	uint8_t rootfs_sha1[SHA1_LEN];
} __attribute__((packed));

typedef uint32_t fdt32_t;

#define FDT_TAGSIZE	sizeof(fdt32_t)
#define FDT_ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define FDT_TAGALIGN(x) (FDT_ALIGN((x), FDT_TAGSIZE))

#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE 0x2
#define FDT_PRO 0x3

struct fdt_node_header {
	fdt32_t tag;
	char name[0];
};

struct fdt_property {
	fdt32_t tag;
	fdt32_t len;
	fdt32_t nameoff;
	char data[0];
};

#define INFO_IMG_NAME "info@1"
#define INFO_IMG_DESC "Firmware meta-information data"

/* binary content of info@1 FIT subimage */
struct info_img_section {
	struct fdt_property desc_prop;
	uint8_t desc_data[32];
	struct fdt_property info_prop;
	struct info_img info;
	struct fdt_property type_prop;
	uint8_t type_data[8];
	struct fdt_property comp_prop;
	uint8_t comp_data[8];
	/* hash@1 - CRC32 */
	struct fdt_node_header hash1_beg;
	uint8_t hash1_name[8];
	struct fdt_property hash1_val_prop;
	uint32_t hash1_val_data;
	struct fdt_property hash1_algo_prop;
	uint8_t hash1_algo_data[8];
	struct fdt_node_header hash1_end;
	/* hash@2 - SHA1 */
	struct fdt_node_header hash2_beg;
	uint8_t hash2_name[8];
	struct fdt_property hash2_val_prop;
	uint8_t hash2_val_data[20];
	struct fdt_property hash2_algo_prop;
	uint8_t hash2_algo_data[8];
	struct fdt_node_header hash2_end;
};

static int populate_info_img(struct cli_options *op)
{
	struct info_img info;
	struct stat kst, rst;
	struct info_img_section *isec;

	uint32_t crc32;

	int kfd = -1, rfd = -1, found = 0;
	void *addr, *ptr;

	kfd = open(op->kernel, O_RDWR);
	if (kfd < 0) {
		fprintf(stderr,
			"failed to open '%s': %s\n", op->kernel, strerror(errno));
		goto err;
	}

	if (fstat(kfd, &kst) < 0) {
		fprintf(stderr,
			"failed to stat '%s': %s\n", op->kernel, strerror(errno));
		goto err;
	}

	rfd = open(op->rootfs, O_RDONLY);
	if (rfd < 0) {
		fprintf(stderr,
			"failed to open '%s': %s\n", op->rootfs, strerror(errno));
		goto err;
	}

	if (fstat(rfd, &rst) < 0) {
		fprintf(stderr,
			"failed to stat '%s': %s\n", op->rootfs, strerror(errno));
		goto err;
	}

	addr = mmap(0, rst.st_size, PROT_READ, MAP_SHARED, rfd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,
			"failed to mmap '%s': %s\n", op->rootfs, strerror(errno));
		goto err;
	}

	memset(&info, 0, sizeof(struct info_img));

	info.magic = htobe32(INFO_IMG_MAG);
	strncpy(info.rev, op->version, INFO_IMG_RV_LEN);
	sha1_csum(addr, rst.st_size, info.rootfs_sha1);

	munmap(addr, rst.st_size);

	addr = mmap(0, kst.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, kfd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,
			"failed to mmap '%s': %s\n", op->kernel, strerror(errno));
		goto err;
	}

	int n=0;

	for (ptr = addr; ptr < (addr + kst.st_size); ptr += 4) {
		n++;
		if (!memcmp(ptr, INFO_IMG_NAME, sizeof(INFO_IMG_NAME))) {
			found = 1;
			break;
		}
	}

	if (!found) {
		fprintf(stderr,
			"FIT info image marker not found!\n");
		goto err_section;
	}

	ptr += FDT_TAGALIGN(sizeof(INFO_IMG_NAME));
	isec = (struct info_img_section *)ptr;

	crc32 = cyg_ether_crc32((unsigned char *)&info, sizeof(struct info_img));
	isec->hash1_val_data = htobe32(crc32);

	sha1_csum((unsigned char *)&info,
		sizeof(struct info_img), isec->hash2_val_data);

	memcpy(&isec->info, &info, sizeof(struct info_img));
	msync(addr, kst.st_size, MS_SYNC|MS_INVALIDATE);
	munmap(addr, kst.st_size);

	close(kfd);
	close(rfd);

	return 0;

err_section:
	munmap(addr, kst.st_size);

err:
	if (kfd != -1)
		close(kfd);
	if (rfd != -1)
		close(rfd);

	return -1;
}

static struct option long_opts[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "kernel", required_argument, NULL, 'k' },
	{ "rootfs", required_argument, NULL, 'r' },
	{ "version", required_argument, NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};

static void usage(const char *exec)
{
	fprintf(stderr, "usage: %s [-h] <-krv>\n"
		"\t-h --help\t\t show help (this message)\n"
		"\t-k --kernel  <path>\t path to fit kernel image\n"
		"\t-r --rootfs  <path>\t path to rootfs image\n"
		"\t-v --version <version>\t version string\n", exec);

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	struct cli_options op;
	int index, opt;

	memset(&op, 0, sizeof(struct cli_options));

	while (1) {
		opt = getopt_long(argc, argv, "hk:r:v:",
			long_opts, &index);

		if (opt == -1)
			break;

		switch (opt)
		{
			case 'k':
				strncpy(op.kernel, optarg, PATH_LEN);
				break;
			case 'r':
				strncpy(op.rootfs, optarg, PATH_LEN);
				break;
			case 'v':
				strncpy(op.version, optarg, VERSION_LEN);
				break;
			default:
				usage(argv[0]);
		}
	}

	if (argc != 7 || optind != 7)
		usage(argv[0]);

	if (populate_info_img(&op))
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}
