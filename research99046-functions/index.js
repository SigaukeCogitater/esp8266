const express = require('express');
const bodyParser = require('body-parser');
const app = express();

app.use(bodyParser.urlencoded({ extended: false }));

app.use(bodyParser.json());

const {Firestore} = require('@google-cloud/firestore');
const PORT = 5555;
const db = new Firestore();

app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});


postData = (req, res) => {
    
    const newData = {

        humidity : req.body.humidity,
        temperature : req.body.temperature,
        createdAt: new Date().toISOString()

    }

    db.collection('liveWeatherData')
        .add(newData)
        .then((doc) => {
            const resData = {
                dataId : doc.id
            };

            res.json(resData);
     
        })
        .catch((err) => {
            res.status(500).json({error: 'something went wrong'});
            console.error(err);
        });

  }

helloWorld = (req, res) => {

    res.send('Hello Word');

}

postDataTwo = (req, res) => {
    
    const newData = {

        humidity : req.params.humidity,
        temperature : req.params.temperature,
        createdAt: new Date().toISOString()

    }

    db.collection('liveWeatherData')
        .add(newData)
        .then((doc) => {
            const resData = {
                dataId : doc.id
            };

            res.json(resData);
     
        })
        .catch((err) => {
            res.status(500).json({error: 'something went wrong'});
            console.error(err);
        });

  }





app.get('/datatwo/:temperature/:humidity', postDataTwo);
app.post('/data', postData);
app.get('/hello', helloWorld);




module.exports = {
    app
};

// exports.api = functions.region('asia-northeast1').https.onRequest(app);