Package: fmt:x64-linux@10.2.1#2

**Host Environment**

- Host: x64-linux
- Compiler: GNU 11.4.0
-    vcpkg-tool version: 2024-04-23-d6945642ee5c3076addd1a42c331bbf4cfc97457
    vcpkg-scripts version: 326d8b43e 2024-05-03 (3 days ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Downloading https://github.com/fmtlib/fmt/archive/10.2.1.tar.gz -> fmtlib-fmt-10.2.1.tar.gz...
-- Extracting source /home/neil/dev/vcpkg/downloads/fmtlib-fmt-10.2.1.tar.gz
-- Applying patch fix-visibility.patch
-- Applying patch fix-write-batch.patch
-- Using source at /home/neil/dev/vcpkg/buildtrees/fmt/src/10.2.1-a991065f88.clean
-- Configuring x64-linux
-- Building x64-linux-dbg
-- Building x64-linux-rel
-- Fixing pkgconfig file: /home/neil/dev/vcpkg/packages/fmt_x64-linux/lib/pkgconfig/fmt.pc
CMake Error at scripts/cmake/vcpkg_find_acquire_program.cmake:166 (message):
  Could not find pkg-config.  Please install it via your package manager:

      sudo apt-get install pkg-config
Call Stack (most recent call first):
  scripts/cmake/vcpkg_fixup_pkgconfig.cmake:193 (vcpkg_find_acquire_program)
  /home/neil/.cache/vcpkg/registries/git-trees/f6f4efa01a5e9ac627f0c6687af8b6b317fbbe42/portfile.cmake:22 (vcpkg_fixup_pkgconfig)
  scripts/ports.cmake:175 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "dependencies": [
    "fmt"
  ]
}

```
</details>
