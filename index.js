var {enumerateDevices} = require('bindings')('dshow_napi');

enumerateDevices((list) => {
  console.log(list);
})