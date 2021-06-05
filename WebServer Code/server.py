from flask import render_template
from flask import Flask
from flask import request
from flaskext.mysql import MySQL

# set up web server
app = Flask(__name__)

# set up connection to local MySQL database
mysql = MySQL()
app.config['MYSQL_DATABASE_USER'] = 'root'
app.config['MYSQL_DATABASE_PASSWORD'] = 'Riders00'
app.config['MYSQL_DATABASE_DB'] = 'cs147'
app.config['MYSQL_DATABASE_HOST'] = 'localhost'
mysql.init_app(app)

@app.route("/insert/")
def insert():
	# get the values from the request
	acceleration = float(request.args.get("acceleration", default="0"))
	time = int(request.args.get("time", default="0")) # in millis
	velocity = float(request.args.get("velocity", default="0"))

	# construct INSERT statement using parameters
	conn = mysql.connect()
	cursor = conn.cursor()
	cursor.execute("INSERT INTO running_times (velocity, time, entry_date) VALUES (" + str(velocity) + ", " + str(time) + ", NOW())")
	conn.commit()
	cursor.close()

	# a success message and copy of the INSERT statement
	return "SUCCESS"

@app.route("/")
@app.route("/home")
def index():
	# get records from database
	conn = mysql.connect()
	cursor = conn.cursor()
	cursor.execute("SELECT * FROM running_times")
	records = cursor.fetchall()

#	for record in records:
#		for val in record:
#			print(type(val), record)
#		print("\n")

	return render_template("index.html", records=records)

@app.route("/edit/", methods=['GET', 'POST'])
def edit():
	if(request.method == 'GET'):
		# get id from request
		rid = int(str(request.args.get("id", default="-1")))

		# bad URL catch
		if(rid == -1):
			return "Bad URL, missing id"
		else:
			# get record from database
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute("SELECT * FROM running_times WHERE rid = " + str(rid) + ";")
			record = cursor.fetchall()

			return render_template("edit.html", record=record[0])
	elif (request.method == 'POST'):
		# get entered fields
		rid = int(str(request.form.get("idField", default="-1")))
		name = str(request.form.get("nameField", default=""))
		comment = str(request.form.get("commentField", default=""))

		# bad url check
		if(rid == -1):
			return "Bad POST"
		else:
			# update record
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute("UPDATE running_times SET name='" + name + "', comment='" + comment + "' WHERE rid=" + str(rid) + ";")
			conn.commit()

			# get record
			cursor.execute("SELECT * FROM running_times WHERE rid=" + str(rid))
			record = cursor.fetchall()

			# pass record to form
			return render_template("edit.html", record=record[0])


@app.route("/graph/")
def graph():
	# get name arg
	runnerName = str(request.args.get("name", default=""))

	# check if name is there
	if(runnerName == ""):
		return "Bad URL"
	else:
		# get records w the name
		conn = mysql.connect()
		cursor = conn.cursor()
		cursor.execute("SELECT * FROM running_times WHERE name LIKE '%" + runnerName + "%'")
		records = cursor.fetchall()

		# render and return html
		return render_template("chart.html", records=records)
