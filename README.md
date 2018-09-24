# Porla

Porla is a programmable BitTorrent client built on top of Node.js. It can be
extended in a variety of ways through plugins and is perfectly suitable for
running as a headless BitTorrent server and other similar setups.

*Porla is currently in its early days - the documentation may be sparse.*


### Features

 - Powerful subscription API to define workflows for various events.
 - Easy to get started - sensible defaults.
 - (Optionally) use a (D)VCS to keep track of changes you make.
 - `git push`-deployments are encouraged!


## Getting started

This example will monitor the `./torrents` directory, add any torrent files it
finds. When a torrent finishes it will send you a push notification via
Pushbullet.


```js
const { Porla }  = require('@porla/porla');
const pushbullet = require('@porla-contrib/pushbullet')(process.env.PUSHBULLET_ACCESS_TOKEN);

// Create the base application. It saves all torrents to the working directory
// and should probably be adjusted to your needs.
const app = new Porla({
    savePath: './'
});

// When a torrent finishes, send a Pushbullet notification. The automation flow
// can be customized with plugins which you can find on NPM or write yourself.
app.subscribe('torrent.finished', [
    pushbullet('Torrent {{ torrent.name }} finished!');
]);

// Monitor and add all torrents added to the directory. The default usage will
// add any files with the '.torrent' extension as well as keep track of
// duplicates to avoid adding the same torrent twice.
app.monitor('./torrents');
```


### On subscriptions

Subscriptions (`porla.subscribe`) is the main way of reacting to events. Each
call to `subscribe` will create a subscription that groups that specific
handler chain.

If one handler returns false, the chain will be cancelled for that invocation,
meaning you can filter quite easily if you (for example) only want to run the
handlers for a torrent that matches a specific regular expression.

Here's a hypothetical example to filter TV shows.

```js
app.subscribe('torrent.finished', [
    // Add a regex filter. The named capture groups will be added to the
    // metadata so we can use them in the following handlers. If it doesn't
    // match the torrent, it will return false and the chain will be skipped.
    Filter.regex(
        // The regex.
        /^(?<show>[a-zA-Z\d\.]+)\.(?<season>S[\d]+)(?<episode>E[\d]+).*$/g

        // The input.
        _ => _.torrent.name,

        // Where should we add the captured groups (if any?)
        _ => _.torrent.metadata
    ),

    // Move the downloaded torrent based on information from the previous regex
    // filter. This will create a neat directory struture for TV shows.
    move('/var/media/TV/{{ torrent.metadata.show }}/{{ torrent.metadata.season }}'),

    // Send some kind of notification to let us know we have a sweet new episode
    // of our favorite show to watch.
    pushbullet('A new episode of {{ torrent.metadata.show }} is available.')
])
```


## Next steps

- [Find plugins](https://www.npmjs.com/search?q=keywords%3Aporla-plugin) to
  make your Porla instance awesome.
- Join the [Mattermost chat](https://im.porla.org) to get help.
- Read the [API documentation](https://docs.porla.org) to help yourself.
