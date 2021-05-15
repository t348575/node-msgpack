# node-msgpack

A native addon for nodejs as a wrapper around [msgpack](https://github.com/t348575/msgpack)

### Build
1)  ```yarn build```
2) Change path of output build files in `lib/binding.js`
3) Include and use `lib/binding.js` like so:

```js
const msgpack = require('./lib/binding');
const fs = require('fs');
const path = require('path');
const dataToParse = {
    name: 'me',
    someData: [
        123,
        'asd',
        {
            location: 'mars',
            x: 12.312341,
            y: 9.1231312
        }
    ]
}
const asMsgpack = msgpack.stringify(dataToParse);
console.log(asMsgpack);
```