# mio — memory item I/O (mem_io)

`mio` lets a program read and write the fields of a C structure **by name,
as text**. You describe the structure once with a table of `Mitem` entries
(field name, type, offset, size, array length, description, format). mio can
then print every field, print one field, print CSV, or parse a text value and
store it into a named field. It is used for shared-memory and memory-mapped
status structures, so an operator can inspect or patch a live structure from
the shell without a purpose-built tool.

## Role in the architecture

mio is the generic "view and edit a struct" layer. Units that keep counters or
state in mapped memory (for example `tp` and `tpi`) define an `Mitem` table
for their struct and ship a small `*mem` tool that maps the file (with `map`)
and hands the pointer and table to mio. mio itself does no mapping, locking or
messaging.

## Public interface (`mem_io.h`)

- **Describing a struct:** `#define STRUCT my_t`, then build
  `Mitem my_l[] = { ..., { END_OF_LIST } };` with macros such as `Ulong`, `Long`,
  `Huge`, `S32`, `U32`, `F32`, `Double`, `String`, `Stamp`, `Stamp32`, `Date`,
  `IP_Addr`, `IP_Port`, bit/mask macros (`Mchar`, `Mlong`, `BitChar`, … with a
  `mask_t` list) and nested `STRUCTURE` entries. The `_` variants
  (`Ulong_(flags, …)`) take `Mflg_t` flags: `INPUT_ONLY`, `DISPLAY_ONLY`,
  `NO_ARRAY_DISPLAY`, `BIT_BASED`.
- **Output:** `Output_Memory_Item(mem, list, name_or_0, prefix)` prints one field,
  or all fields when `name` is 0. `Output_Memory_CSV(...)` prints CSV
  (`CSV_Header` controls the header row).
- **Input:** `Input_Memory_Item(mem, list, name, text)` parses `text` according to
  the field type and stores it.
- **Traversal:** `List_Of_Items` / `Array_Of_Items` walk a table with a callback.
- **Numbers:** `Mio_Int_In/Out` (1–16 bytes, signed/unsigned, base 10/16) and
  `Mio_Float_In/Out` (32/64/80/128-bit; output is the shortest text that reads
  back exactly).
- **Extension:** `MioRegisterHandler(type, input, output)` installs or replaces
  the handler pair for an `Mtyp_t`. Call it before using mio, not during
  concurrent I/O. Returns 0, or -1 with `errno=EINVAL`. Fields with no handler
  report `Func.ERROR` and set `errno=ENOTSUP`.
- `stamp_l` is a ready-made table for `stamp_t` (`struct timeval`).

The `MSGADDR` type and the `MsgAddr` macros are in the header (only when
`msg.h` is included first), but **public mio has no built-in `MSGADDR`
handler**. The header comment refers to a separate `miomsg` module with
`MioMsgInit()`. That module is not part of any public unit. Plain mio is
complete without it.

## Environment

- `MEMORY_ALL`: when set, mio also shows `INPUT_ONLY` fields and accepts input
  to `DISPLAY_ONLY` fields (checked in `mem_core.c`, `mem_in.c`, `mem_out.c`).

## Example (client code)

From `tpi/tpi_l.c` and `tpi/tpimem.c` in the public `tpi` unit:

```c
#define STRUCT tpi_t
Mitem tpi_l[] = {
    Ulong(  Interval, "Interval In Seconds (0=No Interval)" ),
    Ulong(  Count,    "Current Count"                       ),
    Stamp(  Stamp,    "Start Of Next Hour"                  ),
    String( File,     "Hourly Data [ Path & ] File"         ),
    { END_OF_LIST }
};
#undef STRUCT

Input_Memory_Item(  tpi, tpi_l, av[2], av[3] );  /* set field av[2] to av[3] */
Output_Memory_Item( tpi, tpi_l, 0, av[0] );      /* print all fields        */
```

(The tpi table above is shortened; see `tpi_l.c` for all fields.) This unit
has no test programs.

## Dependencies

| Unit | Kind | Evidence |
|------|------|----------|
| `sys` | direct | `mem_io.h` includes `<common.h>` (types `u8`, `u32`, `u_t`, `f_t`, `stamp_t`) |
| `now` | direct | `mem_in.c` and `mem_out.c` include `<now.h>` and call `Micro_From_String`, `Micro_From_Timeval` and `Micro_To_String_Local` for `NOW`/`NOWSECS` fields |

mio does not use `atomic`, `far`, `map` or `sem`. Its makefile has empty
`LIBRARIES`, so programs that link `-lmio` must also link `-lnow -lsys`.

## Getting it

Requires the `base` repository (project root with `BASE`, `rules.mak`,
`targets.mak` and the `repo` unit). A plain `make` in a fresh base clone
fetches the starter list (project `NONE`), which is, in dependency order:

```
sys atomic far map sem now mio tp tpi
```

Or add this unit on its own with `repo add <unit>`.

## License

The original files are offered under `Apache-2.0 OR GPL-2.0-or-later`;
you choose either license. See `LICENSE` and `LICENSES/`. Copyright notices
are retained; third-party-derived files keep their own SPDX identifiers.

## Version

Version tag: `v0.2.0`.
