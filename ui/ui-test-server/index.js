const express = require('express');
var bodyParser = require("body-parser");
const app = express();
app.use(bodyParser.json());

const port = 3000;

var ctx = {
    config : {
        deviceName : 'deviceName',
        wifiSsid : 'wifiSsid',
        wifiPass : 'wifiPass',
        mqttHost : 'mqttHost',
        mqttPort : '1234',
        mqttUser : 'mqttUser',
        mqttPass : 'mqttPass'
    },
    devices : {
        aircond : {
            power : false,
            t : 16,
            mode : 'auto',
            fan : 0,
            swing : 0,
            profile : 0
        }
    }
};

app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
});

app.get('/config', (req, res) => {
    res.send(ctx.config)
    // res.status(404).send(''); // if a fresh installation and there's no config file in the fs
});

app.post('/config', (req, res) => {
    console.log(req.body);
    if (!req.body.deviceName) {
        res.status(400).send('');
        return;
    }
    ctx.config = req.body;
    res.status(200).send();
});

app.get('/devices/aircond', (req, res) => {
    res.send(ctx.devices.aircond)
});

app.post('/devices/aircond', (req, res) => {
    console.log(req.body);
    ctx.devices.aircond = req.body;
    res.status(200).send();
});

app.listen(port, (err) => {
    if (err) {
        return console.log('something bad happened', err)
    }

    console.log(`server is listening on ${port}`)
});
