const assert = require('assert');
const AsyncEventEmitter = require('../lib/asyncEventEmitter');

describe('AsyncEventEmitter', () => {
    describe('on', () => {
        it('should execute the actions sequentially', async () => {
            const emitter = new AsyncEventEmitter();
            let i = 0;

            emitter.on('torrent.added', [
                () => { assert.equal(i, 0); i += 1; },
                () => { assert.equal(i, 1); i += 1; }
            ]);

            emitter.on('torrent.added', [
                () => { assert.equal(i, 2); i += 10; },
                () => { assert.equal(i, 12); i+= 10; }
            ]);

            await emitter.emit('torrent.added', {});

            assert.equal(i, 22);
        });

        it('should stop executing actions when one returns false', async () => {
            const emitter = new AsyncEventEmitter();
            let i = 0;

            emitter.on('torrent.added', [
                () => { return false; },
                () => { assert.fail('Unreachable'); }
            ]);

            emitter.on('torrent.added', [
                () => { i += 10; }
            ]);

            await emitter.emit('torrent.added', {});

            assert.equal(i, 10);
        });

        it('should handle async actions', async function () {
            this.slow(3000);

            const emitter = new AsyncEventEmitter();
            const item = {};

            emitter.on('torrent.added', [
                () => item.before = new Date(),
                async () => new Promise((resolve) => setTimeout(resolve, 1100)),
                () => item.after = new Date()
            ]);

            await emitter.emit('torrent.added', item);

            assert.ok((item.after - item.before) > 1000);
        });
    });
});
