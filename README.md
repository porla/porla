# Porla is a high-performance BitTorrent daemon

Porla is a powerful BitTorrent daemon with high performance and low memory
usage. It is designed for headless operations on servers and seedboxes and can
easily manage tens of thousands of torrents.


## Getting started

Download the latest release and put it somewhere safe. Then, run it.

```sh
$ ./porla
```

To show all options available, suffix with `--help`.

```sh
$ ./porla --help
```

For example, you can run `porla` with an in-memory SQLite database by passing
`--db=:memory:`.

```sh
$ ./porla --db=:memory:
```

## Configuration

You can configure Porla in three ways - environment variables, command line
arguments, and a TOML config file.

Command line arguments have the highest priority, then the config file, and
lastly the environment variables. The app will, however, run without any applied
configuration and use sensible defaults instead.

### Environment variables and command line args

 * `PORLA_CONFIG_FILE` or `--config-file` - path to a TOML config file with
   additional configuration.
 * `PORLA_DB` or `--db` - path a file (which does not need to exist) that `porla`
   will use to store its state.
 * `PORLA_HTTP_BASE_PATH` or `--http-base-path` - set to a path where the HTTP parts
   of Porla will be served. Defaults to `/`.
 * `PORLA_HTTP_HOST` or `--http-host` - set to an IP address which to bind the HTTP
   server. Defaults to _127.0.0.1_.
 * `PORLA_HTTP_METRICS_ENABLED` or `--http-metrics-enabled` - set to true/false to
   enable or disable the metrics endpoint. Defaults to _true_.
 * `PORLA_HTTP_PORT` or `--http-port` - set to the port to use for the HTTP server.
   Defaults to _1337_.
 * `PORLA_LOG_LEVEL` or `--log-level` - the minimum log level to use. Valid values
   are _trace_, _debug_, _info_, _warning_, _error_, _fatal_. Defaults to _info_.
 * `PORLA_SESSION_SETTINGS_BASE` or `--session-settings-base` - the libtorrent
   settings base to use for session settings. Valid values are _default_,
   _min\_memory\_usage_, _high\_performance\_seed_. Defaults to _default_.
 * `PORLA_TIMER_DHT_STATS` or `--timer-dht-stats` - the interval in milliseconds
   to push DHT stats. Defaults to _5000_.
 * `PORLA_TIMER_SESSION_STATS` or `--timer-session-stats` - the interval in
   milliseconds to push session stats. Defaults to _5000_.
 * `PORLA_TIMER_TORRENT_UPDATES` or `--timer-torrent-updates` - the interval in
   milliseconds to push torrent state updates. Defaults to _1000_.

### Config file

```toml
db = ":memory:"
log_level = "info"

[http]
base_path = "/"
host = "127.0.0.1"
metrics_enabled = true
port = 1337

[session_settings]
base = "min_memory_usage"
extensions = [
   "smart_ban",
   "ut_metadata",
   "ut_pex"
]

[timer]
dht_stats = 5000
session_stats = 5000
torrent_updates = 1000
```
