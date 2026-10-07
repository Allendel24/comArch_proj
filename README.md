# LC-2K assembler and simulator

Build the assembler and simulator, then assemble every `.as` program under `asm/`:

```sh
make
```

The executables are written to `build/`. Machine-code files are written to `output/`.
To simulate both sample programs and save their state traces under `output/`, run:

```sh
make run
```

Run one program with `make run-mult` or `make run-comb`. The individual commands are:

```sh
build/assembler asm/mult.as output/mult.mc
build/simulator output/mult.mc
```

`make clean` removes the assembler/simulator binaries and generated sample outputs.

On Windows, use GNU Make with a C++17 compiler such as MinGW-w64 `g++` on `PATH`.
The executables use the `.exe` suffix; PowerShell handles directory creation and cleanup.
