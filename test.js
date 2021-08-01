const {enumerate_devices} = require('./index');

enumerate_devices().then((res) => {
  console.log(res);
}).catch(() => {
  console.log("UnExpectedErrorCaught")
})