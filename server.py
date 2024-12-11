from flask import Flask, request, jsonify, send_from_directory
import subprocess

app = Flask(__name__, static_folder='static')

@app.route('/')
def index():
    return send_from_directory('static', 'index.html')

@app.route('/check', methods=['POST'])
def check_similarity():
    text1 = request.json.get('text1', '')
    text2 = request.json.get('text2', '')

    # Call the C program
    process = subprocess.Popen(
        ['./text_trace'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    input_data = f"{text1}\n{text2}\n"
    stdout, stderr = process.communicate(input=input_data.encode())

    if process.returncode != 0:
        return jsonify({'error': stderr.decode()}), 500

    # Extract similarity percentage from the C program output
    result = stdout.decode().strip()
    return jsonify({'similarity': result})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=10000, debug=False)
