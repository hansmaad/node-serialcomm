const serialcomm = require('../lib');

async function run() {
    try {
        const result = await serialcomm.list();
        console.log(result);
    }
    catch (e) {
        console.log(e);
    }
}

run().then(() => {});
