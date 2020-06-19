# Porla

Porla is a programmable BitTorrent client built for seedboxes and servers. It
has a feature-rich API allowing users to customize their Porla instance as they
wish.

*Porla is currently in its early days - the documentation may be sparse.*

:information_source: Requires Node.js v10.

### Features

 - Built with native bindings to Rasterbar-libtorrent to provide great
   performance and low memory-cost.
 - Easy-to-use API allows users to write plugins and scripts to customize and
   extend Porla.
 - (Optionally) keep Porla in a (D)VCS to track of changes you make.


## Getting started

The easiest way to get started is to use `npm init`.

 - `$ npm init porla`


### Setting up an instance from scratch

Want to do it the old fashioned way? No problem!

- `$ npm install --save porla @porla-contrib/autoadd`

```js
const porla   = require('porla');
const autoadd = require('@porla-contrib/autoadd');

const app = porla({
    plugins: [
        // Set up the autoadd plugin to watch a folder for torrents. Any
        // torrent added here is added to Porla.
        autoadd('/mnt/storage/torrents'),
    ],

    // The default save path where new downloads are saved.
    savePath: '/mnt/storage/downloads'
});

app.on('torrent.finished', ({ torrent }) => [
    // Add event handlers for the torrent.finished event here. Perhaps send a
    // notification or execute another application?
    sendMessage(`Torrent ${torrent.name} finished.`)
]);
```


## Next steps

- [Find plugins](https://www.npmjs.com/search?q=keywords%3Aporla-plugin) to
  make your Porla instance awesome.
- Read the [API documentation](https://docs.porla.org) and explore the Porla
  API.
