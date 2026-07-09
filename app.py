from flask import Flask, render_template, request, jsonify
import subprocess
import json
import os
import platform

app = Flask(__name__)

# ✅ Cross-platform executable name
if platform.system() == "Windows":
    C_EXECUTABLE = "emergency_system.exe"
else:
    C_EXECUTABLE = "./emergency_system"


# -----------------------------
# Utility to run C program
# -----------------------------
def run_c_command(args):
    try:
        result = subprocess.run(
            [C_EXECUTABLE] + args,
            capture_output=True,
            text=True
        )
        # combine stdout + stderr just in case
        return (result.stdout + result.stderr).strip()
    except Exception as e:
        return str(e)


# -----------------------------
# Routes
# -----------------------------

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/report', methods=['POST'])
def report_emergency():
    data = request.json

    raw_type = data.get('type', 'Medical')
    c_type = "Firetruck" if "Fire" in raw_type else "Ambulance"

    run_c_command([
        "add",
        str(data['id']),
        str(data['severity']),
        str(data['location']),
        c_type
    ])

    return jsonify({"status": "success"})


@app.route('/dispatch', methods=['POST'])
def dispatch_unit():
    run_c_command(["dispatch"])
    return jsonify({"status": "success"})


@app.route('/status', methods=['GET'])
def get_status():
    output = run_c_command(["status"])

    try:
        # ✅ extract JSON safely even if extra chars appear
        start = output.find('{')
        end = output.rfind('}') + 1
        clean = output[start:end]

        return jsonify(json.loads(clean))

    except Exception as e:
        return jsonify({
            "error": "Failed to parse C output",
            "raw": output,
            "parse_error": str(e)
        })


@app.route('/reset', methods=['POST'])
def reset_simulation():
    run_c_command(["reset"])
    return jsonify({"status": "reset"})


# ✅ Manual Complete Route (FIXED POSITION)
@app.route('/complete', methods=['POST'])
def manual_complete():
    data = request.json
    run_c_command(["complete", str(data["unitId"])])
    return jsonify({"status": "ok"})


# -----------------------------
# Start Server
# -----------------------------
if __name__ == '__main__':
    if not os.path.exists(C_EXECUTABLE):
        print(f"❌ {C_EXECUTABLE} not found — compile your C file first")
    else:
        app.run(debug=True, port=8000)
