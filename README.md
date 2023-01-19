# Porla is a high-performance BitTorrent client

Porla is a powerful BitTorrent client with high performance and low memory
usage. It is designed for headless operations on servers and seedboxes and can
easily manage tens of thousands of torrents.

The focus of Porla is to provide a BitTorrent client that is _fast_, _correct_
and _extensible_.

### Features

 * [User-defined workflows](https://porla.org/workflows).
 * Support for both BitTorrent v1 and v2.
 * [HTTP API](https://porla.org/api/auth) with JWT auth.
 * Modern web UI.

<p align="center">
   <img src="https://user-images.githubusercontent.com/1491824/213406812-32e16a5c-3d59-4efc-a9f1-d15690ac86e2.png"><br/>
   <i>The main torrents list in Porla.</i>
</p>

## Getting started

Download the latest release and put it somewhere safe. Then, run it. By default,
Porla bind the web UI to `localhost:1337`. On first use you will be prompted to
set up a user account.

```shell
porla
```

To show all options available, suffix with `--help`.

```shell
porla --help
```

For example, you can run `porla` with an in-memory SQLite database by passing
`--db=:memory:`.

```shell
porla --db=:memory:
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
 * `PORLA_HTTP_AUTH_DISABLED_YES_REALLY` - set to `true` to disable HTTP JWT
   authentication (_not recommended_).
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
 * `PORLA_STATE_DIR` or `--state-dir` - a path to a directory where Porla will
   store its state.
 * `PORLA_TIMER_DHT_STATS` or `--timer-dht-stats` - the interval in milliseconds
   to push DHT stats. Defaults to _5000_.
 * `PORLA_TIMER_SESSION_STATS` or `--timer-session-stats` - the interval in
   milliseconds to push session stats. Defaults to _5000_.
 * `PORLA_TIMER_TORRENT_UPDATES` or `--timer-torrent-updates` - the interval in
   milliseconds to push torrent state updates. Defaults to _1000_.
 * `PORLA_WORKFLOW_DIR` or `--workflow-dir` - the path to where Porla will load
   user workflows from.

### Config file



```toml
db = ":memory:"
log_level = "info"
state_dir = "/opt/porla"
workflow_dir = "workflows"

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

## Development

Various bits and pieces of information regarding development.

### Building

We try to make sure Porla is easy to get running directly from the Git
repository. Dependencies are managed with [vcpkg](https://github.com/microsoft/vcpkg).

```shell
git clone --recursive https://github.com/porla/porla
cd porla
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

You can also remove `-G Ninja` if you don't have Ninja available.

### Updating the pre-built Dockerfile build environment

To reduce build times, we use a pre-built Docker layer with all the vcpkg
dependencies already built. This needs to be updated whenever we update the
vcpkg submodule or radically change the project structure.

_Requires push access to the Porla container registry_.

```shell
docker build -t porla-build-env -f Dockerfile.build-env .
docker tag porla-build-env ghcr.io/porla/build-env:<timestamp>
docker push ghcr.io/porla/build-env:<timestamp>
```
