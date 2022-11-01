# Porla(Core) is a high-performance BitTorrent daemon

Porla(Core) is the core process powering Porla. It is a single-binary BitTorrent
daemon that is fully configurable and designed to handle tens of thousands of
torrents.


## Getting started

Porla(Core) can run separate from Porla, and for advanced use cases this is
preferred. The `porlac` binary can easily be set up as a Systemd unit. For a
first run, however, you can run `porlac` directly from the terminal.

To do so, download the latest release and put it somewhere safe. Then, run it.

```sh
$ ./porlac
```

To show all options available, suffix with `--help`.

```sh
$ ./porlac --help
```

For example, you can run `porlac` with an in-memory SQLite database by passing
`--db=:memory:`.

```sh
$ ./porlac --db=:memory:
```

## Configuration

You can configure Porla(Core) in three ways - environment variables, command
line arguments, and a TOML config file.

Command line arguments have the highest priority, then the config file, and
lastly the environment variables. The app will, however, run without any applied
configuration and use sensible defaults instead.

### Environment variables and command line args

 * `PORLA_CONFIG_FILE` or `--config-file` - path to a TOML config file with
   additional configuration.
 * `PORLA_DB` or `--db` - path a file (which does not need to exist) that `porlac`
   will use to store its state.
 * `PORLA_HTTP_AUTH_TOKEN` or `--http-auth-token` - set to any random alphanumeric
   string to enable token authentication for the HTTP server.
 * `PORLA_HTTP_HOST` or `--http-host` - set to an IP address which to bind the HTTP
   server. Defaults to _127.0.0.1_.
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
auth_token = "<random string>"
host = "127.0.0.1"
port = 1337

[session_settings]
base = "min_memory_usage"

[timer]
dht_stats = 5000
session_stats = 5000
torrent_updates = 1000
```
