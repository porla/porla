Installing Porla
================

Porla is distributed as an NPM package and can be installed by simply running
`npm install @porla/porla`. The idea is to be able to run your Porla instance
as a Node.js application, preferrably from a private NPM package.

.. note:: It is recommended to treat your Porla instance as a regular Node.js
          application. Keeping it versioned in Git is encouraged!


Using the Yeoman generator
--------------------------

Yeoman is a scaffolding tool and uses *generators* to create basic scaffolding
for various things. There is an official Porla generator (*generator-porla*)
which can be installed and used to generate a basic Porla instance.

.. code-block:: bash

   $ npm install -g yo generator-porla
   $ yo porla

These two commands will first install Yeoman and the Porla generator into the
global package space, then it will run the generator and set up a basic Porla
instance.


Directly from NPM
-----------------

The following example application will monitor and download each torrent it
finds in the :file:`./torrents` directory.

Install *@porla/porla* and *@porla-contrib/autoadd*.

.. code-block:: bash

   $ npm install @porla/porla @porla-contrib/autoadd

Set up a simple Porla client.

.. code-block:: javascript

   const porla   = require('@porla/porla');
   const autoadd = require('@porla-contrib/autoadd');

   const app = new porla({
       plugins: [
           autoadd('./torrents')
       ]
   });
