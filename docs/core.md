# core

Foundation types every other module builds on. Header root:
`<opus/core/...>`; everything here is also reachable through
`<opus/opus.hpp>`.

## Logging — `<opus/core/log.hpp>`

### Model

A `Logger` is an ordinary value: it pairs a **sink** (where records go)
with a **minimum level** (which records go there). The engine has no
global logger. Whoever needs to log is handed a `Logger&` by whoever owns
it, the same way systems receive every other dependency (CLAUDE.md, "No
hidden state").

```cpp
#include <opus/core/log.hpp>

int main() {
    const opus::Logger log{opus::stderr_sink(), opus::LogLevel::Debug};
    log.debug("loading map {}", "canyon");
    log.warn("{} units lack a sprite", 3);
    return 0;
}
```

### Levels

`LogLevel` is ordered from most to least verbose: `Trace < Debug < Info <
Warn < Error`. A record at level `L` reaches the sink when the logger has a
sink and `L >= min_level()`. The default minimum level is `Info`.

| Level | Use for |
|---|---|
| `Trace` | step-by-step detail, off unless chasing a specific bug |
| `Debug` | diagnostic detail useful during development |
| `Info` | normal lifecycle events |
| `Warn` | something unexpected the engine recovered from |
| `Error` | an operation failed |

`to_string(level)` returns the canonical lowercase name (`"trace"` …
`"error"`), and `"unknown"` for a value outside the enumeration.
`parse_log_level(name)` is its inverse over canonical names only; it is
case-sensitive and does not trim:

| Input | Result |
|---|---|
| a canonical name | that `LogLevel` |
| `""` | `ParseLogLevelError::Empty` |
| anything else (`"WARN"`, `" warn"`, `"verbose"`) | `ParseLogLevelError::Unknown` |

### Writing records

- `trace` / `debug` / `info` / `warn` / `error(fmt, args...)` and
  `log(level, fmt, args...)` take a `std::format` string, checked at
  compile time. Formatting happens only when the level is enabled, so a
  disabled record costs a sink check and a level comparison.
- `write(level, message)` forwards an already-formatted string verbatim;
  braces in it are not interpreted.
- `enabled(level)` reports whether a record at `level` would reach the
  sink, for callers that want to skip expensive preparation.

### Sinks

`LogSink` is `std::function<void(LogLevel, std::string_view)>`. The
message view is valid only for the duration of the call; a sink that keeps
records copies them.

- An **empty sink** discards every record: `enabled()` is false for all
  levels and nothing is formatted.
- `stderr_sink()` writes one line per record, `[<level>] <message>`, with a
  single write call per record. A failed write is dropped: there is nowhere
  left to report it.

### Threads

A `Logger` does no locking. Concurrent calls to its `const` members are
safe when the sink is; `set_min_level` must not race with anything else on
the same logger. `stderr_sink` is safe to call concurrently, and records
from different threads never interleave within a line.

### Out of scope

Log files, rotation and in-game consoles are sinks that the modules owning
those concerns provide (`platform`, `devtools`); `core` defines only the
contract they plug into.
