var {enumerateDevices} = require('bindings')('dshow_api');

const enumerate_devices = () => (new Promise((resolve, reject) => {
  enumerateDevices((res) => {
    if (res.length > 0)
      resolve(res);
    else reject()
  })
}))

module.exports = {enumerate_devices}