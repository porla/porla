Configuration
=============

When creating the Porla instance you can provide an object with settings that
changes how Porla runs.


The config object
-----------------

This is a complete list of the settings you can provide to Porla.

.. code-block:: javascript

   {
       // The path where Porla stores its data files, such as session state,
       // torrent files, etc. Defaults to '%CWD%/data'.
       dataPath: '/opt/porla/data',

       // Disable DHT
       dht: false,

       // Enable DHT with custom bootstrap nodes
       dht: {
           // The nodes to use when bootstrapping the DHT. If unspecified,
           // will default to the following.
           // - router.utorrent.com:6881
           // - router.bittorrent.com:6881
           // - dht.transmissionbt.com:6881
           // - dht.aelitis.com:6881
           bootstrapNodes: [
               [ 'router.utorrent.com', 6881 ]
           ]
       },

       // Specifies the listen interfaces (and ports) we should use when
       // listening for incoming connections. Defaults to 0.0.0.0:6881.
       listenInterfaces: [
           [ '0.0.0.0', 6881 ]
       ],

       // A list of plugins that should attach to Porla.
       plugins: [
       ],

       // Enable SOCKS5 proxy
       proxy: {
           type: porla.SOCKS5,
           host: '127.0.0.1',
           port: 1080
       },

       // Set to true to require encryption for peer connections. Defaults to
       // false.
       requireEncryption: false,

       // The path where we should save downloads. Defaults to '%CWD%':
       savePath: '/mnt/media/downloads'
   }
