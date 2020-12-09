import utils
from flask import Flask, render_template, request

app = Flask(__name__)
bluetooth_address = None


@app.route('/')
def main_page():
    return render_template("main.html")
# TODO: main.html 뽀샵질 해야됨.
# TODO: main.js로 돌려서 어떻게 좀 보기 좀 좋게좀 해야할거 같음.


@app.route('/get_data')
def get_data():
    global bluetooth_address
    print(bluetooth_address)
    if bluetooth_address is None:
        while bluetooth_address is None:
            bluetooth_address = utils.get_address()
        return "We now find Server address!\n Wait for data!"
    else:
        res_list = utils.get_data(bluetooth_address)
        print(res_list)
        res_data = str(res_list[0][0]) + "\n" + str(res_list[1][0])
        return res_data


@app.route('/send_data')
def send_data():
    global bluetooth_address
    if bluetooth_address is None:
        while bluetooth_address is None:
            bluetooth_address = utils.get_address()
    control = request.args.get('control')
    res = 0
    if control == 'temp':
        command = request.args.get('command')
        actual_command = 0
        if command == 'fahrenheit':
            actual_command = 1
        res = utils.send_lcd(bluetooth_address, actual_command, '')
    if control == 'data':
        data = request.args.get('command')
        print(data)
        res = utils.send_lcd(bluetooth_address, 0, data)
    if control == 'led':
        data = request.args.get('command')
        list_data = data.split('|')
        res = utils.send_led(bluetooth_address, list_data)
    print(str(res))
    return str(res)


if __name__ == '__main__':
    app.run()
