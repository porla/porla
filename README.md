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

 * `PORLA_DB` or `--db` - path a file (which does not need to exist) that `porlac`
   will use to store its state.


### Config file

```toml
db = ":memory:"
```
