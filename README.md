# zlib Unikraft port

Build unmodified zlib 1.3.2 from the pinned, verified release archive.
Config.uk, Makefile.uk and Library.uk describe a general-purpose compression
library. No nghttpx, QUIC or network library is required.

CONFIG_LIBZLIB requires HAVE_LIBC. The compression, decompression and checksum
APIs do not select a filesystem. CONFIG_LIBZLIB_GZFILE adds the gz* file APIs
when HAVE_VFS is available and defaults on in that configuration. Without it,
gzip-format in-memory deflate/inflate still work; gz* file symbols are omitted.
The port leaves the VFS implementation, filesystem and mounts to the application.
It no longer forces the old vfscore/ramfs/devfs combination. Existing full-libc
applications with a VFS retain gz* APIs by default. musl is the qualified libc;
HAVE_LIBC is the interface requirement, not a claim every libc has been tested.

The independent tests/contract application checks memory compression, streaming
gzip, corrupted input and (when enabled) gzip files on a private RAM filesystem.
Run it with gzip file support enabled and disabled. Successful HTTP/3 linking is
not used as a substitute for these direct API tests.

Source archive/hash and upstream Zlib licensing are recorded in Library.uk and
COPYING.upstream; the original port Git history is retained.

## Independent build and contract run

Set UK_ROOT, MUSL_PORT and PORT to separate core, libc and this port checkouts;
use fresh absolute OUT and CFG paths. No nghttpx repository is needed.

```sh
make -C "$UK_ROOT" A="$PORT/tests/contract" O="$OUT" C="$CFG" \
  L="$MUSL_PORT:$PORT" UK_DEFCONFIG="$PORT/tests/contract/defconfig" \
  COMPILER=clang-19 defconfig
make -C "$UK_ROOT" A="$PORT/tests/contract" O="$OUT" C="$CFG" \
  L="$MUSL_PORT:$PORT" COMPILER=clang-19 -j8
qemu-system-aarch64 -machine virt,gic-version=host -cpu host -accel kvm \
  -m 512M -smp 1 -display none -monitor none -serial stdio -no-reboot \
  -nic none -kernel "$OUT/contract_qemu-arm64"
```

For the memory-only case, copy tests/contract/defconfig to a private file,
append `# CONFIG_LIBZLIB_GZFILE is not set`, pass that file as UK_DEFCONFIG
and use new OUT/CFG paths. The same test must report gzip_file=0; the gz* file
symbols must be absent from libzlib.o.
