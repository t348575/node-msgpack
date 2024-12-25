# node-msgpack ![Visits](https://lambda.348575.xyz/repo-view-counter?repo=node-msgpack)

A native addon for nodejs as a wrapper around [msgpack](https://github.com/t348575/msgpack)

### Build
1)  ```yarn build```
2) Change path of output build files in `lib/binding.js`
3) Include and use `lib/binding.js` like so:

```js
const msgpack = require('./lib/binding');
const dataToParse = {
    name: 'me',
    alive: false,
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
