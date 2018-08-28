const assert = require('assert');
const { Porla } = require('../');

describe('Porla', () => {
    describe('subscribe', () => {
        it('should execute the actions sequentially', async () => {
            const porla = new Porla();
            let i = 0;

            porla.subscribe('torrent.added', [
                () => { assert.equal(i, 0); i += 1; },
                () => { assert.equal(i, 1); i += 1; }
            ]);

            porla.subscribe('torrent.added', [
                () => { assert.equal(i, 2); i += 10; },
                () => { assert.equal(i, 12); i+= 10; }
            ]);

            await porla.emit('torrent.added', {});

            assert.equal(i, 22);
        });

        it('should stop executing actions when one returns false', async () => {
            const porla = new Porla();
            let i = 0;

            porla.subscribe('torrent.added', [
                () => { return false; },
                () => { assert.fail('Unreachable'); }
            ]);

            porla.subscribe('torrent.added', [
                () => { i += 10; }
            ]);

            await porla.emit('torrent.added', {});

            assert.equal(i, 10);
        });

        it('should handle async actions', async function () {
            this.slow(3000);

            const porla = new Porla();
            const item = {};

            porla.subscribe('torrent.added', [
                () => item.before = new Date(),
                async () => new Promise((resolve) => setTimeout(resolve, 1100)),
                () => item.after = new Date()
            ]);

            await porla.emit('torrent.added', item);

            assert.ok((item.after - item.before) > 1000);
        });
    });
});
