/* 
Sources at:
https://github.com/carlobiermann/lora-mesh/tree/main/02_random_data_to_db#sources
*/

const Pool = require('pg').Pool
const pool = new Pool({
	user: '',
	host: '',
	database: '',
	port:  ,
})


var obj = {};


const createEntry = (request, response) => {
	const {nodeID, hops, lat, lon, date} = request.body
	console.log(request.body);
	pool.query('UPDATE nodes SET hops = $2, lat = $3, lon = $4, date = $5 WHERE nodeid = $1', [nodeID, hops, lat, lon, date], (error, results) => {
		if (error) {
			throw error
		}
		response.status(201).send(request.body);
})
	pool.query('SELECT * FROM nodes ORDER BY nodeid', (error, results) => {
		if (error) {
			throw error
		}
		console.table(results.rows);
	})
}

module.exports = { 
	createEntry
}
