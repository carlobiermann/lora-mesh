## Random data to db 

**This system inserts a simulated GPS payload from a T-Beam into a database via LoRa and TCP/IP.**

![meshdb-simulated](/pics/meshdb-simulated.png)

## Table of Contents

- [Description](#description)
- [How to start program](#how-to-start-program)
- [Sources](#sources)
- [Stuff to change](#stuff-to-change)

## Description

*Current setup:*
- 1x LoRa Node -> T-Beam 
- 1x Gateway Node -> T-Beam 
- 1x Socket Server -> Running on localhost:8000
- 1x Express Server -> Running on localhost:3000 
- 1x PostgreSQL Database -> Running on localhost:5432

*The data transmission:*
1. LoRa Node transmits random/simulated **payload** to the **Gateway Node**:

- Payload is a 24 byte array:
  - nodeId -> int -> 4 bytes
  - number of LoRa hops -> int -> 4 bytes
  - lat  -> double -> 8 bytes
  - lon -> double -> 8 bytes

2. Gateway Node receives the payload and forwards the byte array via a socket/TCP/IP **client** instance to the **Socket Server**.

3. Socket Server receives the byte array. When the client disconnects, the payload and a date/timestamp are formatted as a blob of JSON. This JSON blob is sent to the **Express Server** via a **POST Request**.

4. The Express Server receives the JSON blob and inserts its data into the **PostgreSQL Database** via an SQL query. The terminal of the Express Server then displays the contents of the database

#### How to start program 

*1. Initialize Database:*

Open terminal:

*Start* database: 
`brew services start postgresql`

or

*Stop* database: 
`brew services stop postgresql`

Open new terminal window: 

`psql [database name]`

*2. Start Express Server:*

`cd ES`

Enter the database details in **app.js** and **queries.js**:

```
const Pool \= require('pg').Pool
const pool \= new Pool({
user: ' ',
host: ' ',
database: ' ',
port: 
}) 
```

If not already installed: 

```
npm install
node app.js
```

*3. Start Socket Server:*

Open new terminal window/tab: 

```
cd SS
./main
```

*4. Start Gateway Node (GW):*

`cd GW`

Open GW sketch in Arduino IDE and enter the local Wifi network name and password into: 

`WiFiMulti.addAP("", "");`

Upload the sketch 


*5. Start LoRa Node (LN):*

`cd LN`

Open LN sketch in Arduino IDE and upload the sketch


## Sources

*A list of sources and adopted functions each program is based on:*

*[ES](/ES):*
- [PostgreSQL Documentation](https://www.postgresql.org/docs/8.0/tutorial-arch.html)
- [Node.js, Express.js, and PostgreSQL: CRUD REST API example](https://blog.logrocket.com/nodejs-expressjs-postgresql-crud-rest-api-example/)
- [How To Display Data From MySQL Database Table In Node.Js](https://codingstatus.com/how-to-display-data-from-mysql-database-table-in-node-js/)
- [Stack Overflow: How do I consume the JSON POST data in an Express application](https://stackoverflow.com/questions/10005939/how-do-i-consume-the-json-post-data-in-an-express-application)
- [Step by Step Javascript and Postgres Tutorial using node-postges](https://www.youtube.com/watch?v=ufdHsFClAk0)
- [Tables Generator](https://www.tablesgenerator.com/html_tables)
- [OpenStreetMap & OpenLayers: Creating A Map With Markers](https://mediarealm.com.au/articles/openstreetmap-openlayers-map-markers/)
- [3 ways to display two divs side by side (float, flexbos, CSS grid)](https://coder-coder.com/display-divs-side-by-side/)


*[GW](/GW):*
- [Espressif: WifiClientBasic Example](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientBasic/WiFiClientBasic.ino)
- [LoRa.h example: LoRaSimpleGateway](https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaSimpleGateway/LoRaSimpleGateway.ino)

*[LN](/LN):*
- [LoRa.h example: LoRaSimpleNode](https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaSimpleNode/LoRaSimpleNode.ino)
- [Arduino Forum: printing a double variable](https://forum.arduino.cc/index.php?topic=44216.0)

*[SS](/SS):*
- [How to Code a Server and Client in C with Sockets on Linux - Code Examples](https://www.binarytides.com/server-client-example-c-sockets-linux/)
- [curltest.c](https://gist.github.com/amichaelgrant/080ecdc254fc551c8f6f)
- [json-c library](https://github.com/json-c/json-c)
- [Stack Overflow: passing array as function argument in c](https://stackoverflow.com/questions/35699808/passing-array-as-function-argument-c)
- [C library function - memcpy()](https://www.tutorialspoint.com/c_standard_library/c_function_memcpy.htm)
- [Stack Overflow: C header files and compilation/linking](https://stackoverflow.com/questions/18548157/c-header-files-and-compilation-linking)
- [Stack Overflow: Get the current time in C](https://stackoverflow.com/a/18635954)

## Stuff to change

- Switch from a simulated payload to *real* GPS data
- Displaying the real-time GPS coordinates on a map with markers 
- Render the contents of the database to an HTML table
- **Implementing a Mesh Routing Algorithm to route payloads from *multiple* LoRa Nodes**
