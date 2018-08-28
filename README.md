# Porla

Porla is a programmable BitTorrent client written in Node.js. It is easily
customized with community (or your own) plugins.


## Getting started

```js
const { Porla }  = require('@porla/porla');
const pushbullet = require('@porla/pushbullet')(process.env.PUSHBULLET_ACCESS_TOKEN);

// Create the base application. It saves all torrents to the working directory
// and should probably be adjusted to your needs.
const app = new Porla({
    savePath: './'
});

// When a torrent finishes, send a Pushbullet notification. The automation flow
// can be customized with plugins which you can find on NPM or write yourself.
app.on('torrent.finished', [
    pushbullet('Torrent {{ name }} finished!');
]);

// Monitor and add all torrents added to the directory. The default usage will
// add any files with the '.torrent' extension as well as keep track of
// duplicates to avoid adding the same torrent twice.
app.monitor('./torrents');
```


## Next steps

- Read the [API documentation](https://docs.porla.org).
- Join the [Mattermost chat](https://im.porla.org).
