/*
 *  partition hacks
 *
 *  Copyright (c) 2015 xerub
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * boot ramdisk via kloader
 * mount_hfs /dev/disk0s1s2 /mnt1
 * DYLD_INSERT_LIBRARIES=/usr/lib/libpatch.dylib /usr/libexec/ramrod/ramrod
 */

#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#if 1
#define dbg(args...) /*printf(args)*/
#else
#define dbg my_printf
#endif

#define MY_SPARE_SIZE (2 * 1024 * 1024 * 1024)
#define DLSYM(sym) *(void **)&sym = dlsym(h, #sym)

typedef void (*logger_t)(const char *fmt, va_list ap);
typedef void (*callback_t)(char *buf, ssize_t size, void *opaque);
typedef void (*executor_t)(char *const *args, callback_t callback, void *opaque);

static void __attribute__((format(printf, 1, 2)))
my_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fflush(stderr);
}

static void
my_logger(const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	fflush(stderr);
}

static void
my_execute_command_with_callback(char *const *args, callback_t callback, void *opaque)
{
	pid_t pid;
	int status;
	int pip[2];
	ssize_t size;
	char buffer[1024];

	dbg("executing %s\n", *args);
	if (pipe(pip) == -1) {
		dbg("pipe failed while preparing to execute %s: %s\n", *args, strerror(errno));
		return;
	}
	pid = fork();
	if (pid == -1) {
		dbg("fork %s failed: %s\n", *args, strerror(errno));
		close(pip[0]);
		close(pip[1]);
		return;
	}
	if (!pid) {
		if (dup2(pip[1], STDOUT_FILENO) == -1) {
			_exit(1);
		}
		if (dup2(pip[1], STDERR_FILENO) == -1) {
			_exit(2);
		}
		close(pip[0]);
		if (execv(*args, args) == -1) {
			dbg("execv: %s: %s\n", *args, strerror(errno));
			_exit(1);
		}
	}
	close(pip[1]);
	while ((size = read(pip[0], buffer, sizeof(buffer) - 1)) > 0) {
		buffer[size] = '\0';
		callback(buffer, size, opaque);
	}
	if (waitpid(pid, &status, 0) == -1) {
		dbg("waitpid failed for %s: %s\n", *args, strerror(errno));
		close(pip[0]);
		return;
	}
	if (!WIFEXITED(status)) {
		dbg("%s terminated abnormally\n", *args);
	}
	close(pip[0]);
	return;
}

static void *
my_dlsym(void *h, const char *name)
{
	void (*ramrod_register_plugin)(void **ptr);
	void (*set_partition_logging_function)(logger_t logger);
	void (*set_partition_execution_function)(executor_t executor);
	void (*set_partition_execution_logging_function)(logger_t logger);
	void (*create_update_partition)(unsigned long long size, char *update_mnt, char *data_mnt);
	*(void **)&ramrod_register_plugin = dlsym(h, name);
	DLSYM(set_partition_logging_function);
	DLSYM(set_partition_execution_function);
	DLSYM(set_partition_execution_logging_function);
	DLSYM(create_update_partition);
	if (ramrod_register_plugin && create_update_partition) {
		void *ptr;
		ramrod_register_plugin(&ptr);
		my_printf("---\n");
		set_partition_logging_function(my_logger);
		set_partition_execution_function(my_execute_command_with_callback);
		set_partition_execution_logging_function(my_logger);
		create_update_partition(MY_SPARE_SIZE, "/mnt2", "/mnt1");
		my_printf("===\n");
		usleep(1000000);
		exit(1);	/* server will reboot */
	}
	return (void *)ramrod_register_plugin;
}


/*** hooking engine **********************************************************/


struct import_t {
	unsigned long a;
	const char *s;
};


static int
find_stubs(const char *p, struct import_t *table)
{
	unsigned int i;
	const struct mach_header *hdr = (struct mach_header *)p;
	const char *q;
	int is64 = 0;

	const char *linkEditBase = NULL;
	intptr_t slide = 0;

	unsigned int *indirect_symbols = NULL;
	unsigned long nindirect_symbols = -1;
	struct nlist *symbols = NULL;
	unsigned long nsymbols = -1;
	const char *strings = NULL;
	unsigned long strings_size = -1;

	int found = 0;

	if ((hdr->magic & ~1) != 0xFEEDFACE) {
		return -1;
	}
	if (hdr->magic & 1) {
		is64 = 4;
	}
	/* XXX check hdr->filetype? */

	q = p + sizeof(struct mach_header) + is64;
	for (i = 0; i < hdr->ncmds; i++) {
		const struct load_command *cmd = (struct load_command *)q;
		uint32_t c = cmd->cmd;
		if (c == LC_SEGMENT) {
			const struct segment_command *seg = (struct segment_command *)q;
			if (!strcmp(seg->segname, "__TEXT")) {
				slide = (intptr_t)(p - seg->vmaddr);
			} else if (!strcmp(seg->segname, "__LINKEDIT")) {
				linkEditBase = (char *)(seg->vmaddr + slide - seg->fileoff);
			}
		}
		if (c == LC_SEGMENT_64) {
			const struct segment_command_64 *seg = (struct segment_command_64 *)q;
			if (!strcmp(seg->segname, "__TEXT")) {
				slide = (intptr_t)(p - seg->vmaddr);
			} else if (!strcmp(seg->segname, "__LINKEDIT")) {
				linkEditBase = (char *)(seg->vmaddr + slide - seg->fileoff);
			}
		}
		q = q + cmd->cmdsize;
	}

	if (!linkEditBase) {
		return -1;
	}

	q = p + sizeof(struct mach_header) + is64;
	for (i = 0; i < hdr->ncmds; i++) {
		const struct load_command *cmd = (struct load_command *)q;
		uint32_t c = cmd->cmd;
		if (c == LC_DYSYMTAB) {
			const struct dysymtab_command *dys = (struct dysymtab_command *)q;
			indirect_symbols = (unsigned int *)&linkEditBase[dys->indirectsymoff];
			nindirect_symbols = dys->nindirectsyms;
		}
		if (c == LC_SYMTAB) {
			const struct symtab_command *sym = (struct symtab_command *)q;
			symbols = (struct nlist *)&linkEditBase[sym->symoff];
			nsymbols = sym->nsyms;
			strings = &linkEditBase[sym->stroff];
			strings_size = sym->strsize;
		}
		q = q + cmd->cmdsize;
	}

	if (indirect_symbols == NULL || symbols == NULL || strings == NULL) {
		return -1;
	}

	q = p + sizeof(struct mach_header) + is64;
	for (i = 0; i < hdr->ncmds; i++) {
		const struct load_command *cmd = (struct load_command *)q;
		uint32_t c = cmd->cmd;
		if (c == LC_SEGMENT) {
			unsigned int j, k;
			const struct segment_command *seg = (struct segment_command *)q;
			const struct section *sec = (struct section *)(q + sizeof(struct segment_command));
			for (j = 0; j < seg->nsects; j++) {
				unsigned long stride, count, n;
				if ((sec[j].flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS) {
					stride = sizeof(int);
				} else {
					continue;
				}
				count = sec[j].size / stride;
				n = sec[j].reserved1;
				for (k = 0; k < count && n + k < nindirect_symbols; k++) {
					struct import_t *t;
					unsigned int a = sec[j].addr + k * stride;
					unsigned long z = indirect_symbols[k + n];
					if (z & (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS)) {
						continue;
					}
					if (z >= nsymbols || (unsigned long)symbols[z].n_un.n_strx >= strings_size) {
						continue;
					}
					for (t = table; t->s != NULL; t++) {
						if (t->a == 0 && !strcmp(t->s, strings + symbols[z].n_un.n_strx)) {
							t->a = a;
							found++;
							break;
						}
					}
				}
			}
		}
		if (c == LC_SEGMENT_64) {
			unsigned int j, k;
			const struct segment_command_64 *seg = (struct segment_command_64 *)q;
			const struct section_64 *sec = (struct section_64 *)(q + sizeof(struct segment_command_64));
			for (j = 0; j < seg->nsects; j++) {
				unsigned long stride, count, n;
				if ((sec[j].flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS) {
					stride = sizeof(long);
				} else {
					continue;
				}
				count = sec[j].size / stride;
				n = sec[j].reserved1;
				for (k = 0; k < count && n + k < nindirect_symbols; k++) {
					struct import_t *t;
					unsigned long a = sec[j].addr + k * stride;
					unsigned long z = indirect_symbols[k + n];
					struct nlist_64 *symbolz = (struct nlist_64 *)symbols;
					if (z & (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS)) {
						continue;
					}
					if (z >= nsymbols || (unsigned long)symbolz[z].n_un.n_strx >= strings_size) {
						continue;
					}
					for (t = table; t->s != NULL; t++) {
						if (t->a == 0 && !strcmp(t->s, strings + symbolz[z].n_un.n_strx)) {
							t->a = a;
							found++;
							break;
						}
					}
				}
			}
		}
		q = q + cmd->cmdsize;
	}
	return found;
}


/*** entrypoints *************************************************************/


static void __attribute__((constructor))
init_(void)
{
	struct import_t table[] = {
		{ 0, "_dlsym" },
		{ 0, NULL }
	};
	int rv = find_stubs((void *)_dyld_get_image_header(0), table);
	intptr_t slide = _dyld_get_image_vmaddr_slide(0);
	if (rv == 1 && table[0].a) {
		my_printf("hooked %lx %s\n", table[0].a, table[0].s);
		*(void **)(table[0].a + slide) = (void *)my_dlsym;
	}
}
