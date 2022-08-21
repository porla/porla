# Porla - the _ultimate_ BitTorrent seedbox

Porla is a highly customizable BitTorrent client built specifically for servers
and seedboxes. It builds on Rasterbar-libtorrent and contains native Node.js
bindings for maximum performance and minimal memory usage.

:information_source: Requires Node.js v16.

### Features

 - Supports BitTorrent 2.0 (BEP 52) and all other modern BitTorrent features
   such as DHT, UPnP, and more.
 - Built with native bindings to Rasterbar-libtorrent to provide great
   performance and low memory usage.
 - Easy-to-use API allows users to write plugins and scripts to customize and
   extend Porla.
 - (Optionally) keep Porla in version control to track of changes you make.


## Getting started

If you just want to kick the wheels and see if Porla is for you, there is no
need to install anything other than Node.js (v16).

Simply run `npx porla` to download and run the latest version of Porla.

If you want to set up a permanent application instance, it's almost as easy.

```sh
# Run the `create-porla` package to set up a Porla instance.
npm create porla

# In the created directory, start Porla.
npm start
```


## Next steps

- [Find plugins](https://www.npmjs.com/search?q=keywords%3Aporla-plugin) to
  make your Porla instance awesome.
- Read the [API documentation](https://docs.porla.org) and explore the Porla
  API.
