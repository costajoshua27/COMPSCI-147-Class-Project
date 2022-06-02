from flask import Flask
from flask import request
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

data = {}

@app.route("/")
def hello():
  _id = request.args.get("id")
  charge = request.args.get("charge")
  timestamp = request.args.get("timestamp")

  if _id not in data:
      data[_id] = []

  data[_id].append({ "charge": charge, "timestamp": timestamp })

  return "Success"

@app.route("/data")
def hello2():
  _id = request.args.get("id")

  return { "result": data[_id] }