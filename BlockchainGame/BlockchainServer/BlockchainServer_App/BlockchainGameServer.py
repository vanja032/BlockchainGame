import subprocess
import json
import time
from flask import *
from flask_cors import CORS, cross_origin
from encrypt import AesEncryption
from waitress import serve
import re

encrypt = AesEncryption()
app = Flask(__name__)
CORS(app)
app.config['CORS_HEADERS'] = 'Content-Type'

private_key = "<game_account_private_key"

regex = r'\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b'

@app.route('/insertscore', methods=['POST', 'PUT'])
@cross_origin()
def ParseContracts():
    try:
        data = str(request.data.decode("utf-8"))
        #print(data)
        print(encrypt.decrypt(data, private_key))
        data = json.loads(encrypt.decrypt(data, private_key))
        post_data(data["account"], data["action"], data["data"])
        return "True"
    except Exception as ex:
        print(ex)
        return "False"

@app.route('/subscribe', methods=['POST', 'PUT'])
@cross_origin()
def SubscribeAccount():
    try:
        #print(data)
        subscribe(request.data.decode("utf-8"))
        return "True"
    except Exception as ex:
        #print(ex)
        return str(ex)


def post_data(account, action, data):
    subprocess.run(["cline", "wallet", "unlock", "--password", "<wallet_password>"])
    username = data["username"]
    score = data["score"]
    coins = data["coins"]
    subprocess.run(["cline", "push", "action", str(account), str(action), "[\"" + username + "\", \"" + score + "\", \"" + coins + "\"]", "-p", account + "@active"])

def subscribe(email):
    emails = []
    if not re.fullmatch(regex, email):
        raise Exception("Email is not valid")
    with open("subscriptions.json", "r") as file:
        data = file.read()
        if data == "":
            data = "[]"
        emails = json.loads(data)
    if email not in emails:
        with open("subscriptions.json", "w") as file:
            emails.append(email)
            json.dump(emails, file)
    else:
        raise Exception("Email already exist")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port="443", ssl_context=("<cert.pem>", "<privkey.pem>"))
