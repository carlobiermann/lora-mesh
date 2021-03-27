const express = require('express')
const bodyParser = require('body-parser')
const app = express()
const port = 3000
const db = require('./queries')


var path = require('path');

//uncomment


const Pool = require('pg').Pool
const pool = new Pool({
	user: '',
	host: '',
	database: '',
	port: 
})


//app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');

app.use(bodyParser.json())
app.use(
	bodyParser.urlencoded({
		extended: true,
	})
)

app.post('/', db.createEntry)

app.get('/', function(req, res){
	pool.query('SELECT * FROM nodes ORDER BY nodeid', (error, results) => {
		if(error) {
			throw error
		}
		console.table(results.rows);
	})
	res.sendFile(path.join(__dirname + '/index.html'))

})

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})


